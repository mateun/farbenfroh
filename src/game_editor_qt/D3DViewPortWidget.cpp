//
// Created by mgrus on 16.04.2025.
//

#include "D3DViewPortWidget.h"

#include <engine/dx11/dx11_api.h>

void D3DViewPortWidget::uploadGridData() {
    std::vector<glm::vec3> gridLines;
    int halfSize = 10;
    float spacing = 1.0f;

    for (int i = -halfSize; i <= halfSize; ++i) {
        // Vertical line
        gridLines.push_back(glm::vec3(i * spacing, 0, -halfSize * spacing));
        gridLines.push_back(glm::vec3(i * spacing, 0,  halfSize * spacing));

        // Horizontal line
        gridLines.push_back(glm::vec3(-halfSize * spacing, 0, i * spacing));
        gridLines.push_back(glm::vec3( halfSize * spacing, 0, i * spacing));
    }

    grid_vertex_buffer = dx11_createVertexBuffer(gridLines);
}

D3DViewPortWidget::D3DViewPortWidget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    HWND hwnd = reinterpret_cast<HWND>(winId());
    dx11_init(hwnd);
    ID3DBlob* vsBlob;
    ID3DBlob* psBlob;
    auto vs_blob_result = dx11_compileShader(L"../src/game_editor_qt/shaders/uber_shader.hlsl", &vsBlob, "VSMain" , "vs_5_0");
    auto ps_blob_result = dx11_compileShader(L"../src/game_editor_qt/shaders/uber_shader.hlsl", &psBlob, "PSMain" , "ps_5_0");
    if (!vs_blob_result || !ps_blob_result) {
        exit(12344);
    }
    dx11_createVertexShaderFromByteCode(vsBlob, &uber_vertex_shader_);
    //uber_vertex_shader_  = dx11_createVertexShaderFromByteCode(vsBlob);
    uber_pixel_shader_ = dx11_createPixelShaderFromByteCode(psBlob);
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    dx11_createInputLayout(layoutDesc, 1, vsBlob, &uber_input_layout_);

    uploadGridData();

}

void D3DViewPortWidget::paintEvent(QPaintEvent *paint_event) {
    dx11_clearBackbuffer({1, 0, 1, 1});

    // The actual rendering of our 3d contents go here!
    // For now just render a grid!
    dx11_setShaderAndInputLayout(uber_input_layout_, uber_vertex_shader_, uber_pixel_shader_);

    UINT stride = sizeof(glm::vec3);
    UINT offset = 0;
    dx11_drawFromVertexBuffer(grid_vertex_buffer, stride, offset);

    dx11_presentBackbuffer();
}

QPaintEngine * D3DViewPortWidget::paintEngine() const {
    return nullptr; // disable Qt painting (important!)
}
