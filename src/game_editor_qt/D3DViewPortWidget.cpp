//
// Created by mgrus on 16.04.2025.
//

#include "D3DViewPortWidget.h"

#include <engine/dx11/dx11_api.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

/**
 * Used to update the main transform matrix for every object, every frame.
 * @param worldViewProj The updated, new main transform matrix
 */
void D3DViewPortWidget::updateWorldViewProjectionMatrix(const glm::mat4& worldViewProj) {
    FrameTransform data;
    data.worldViewProj = worldViewProj;

    D3D11_MAPPED_SUBRESOURCE mapped;
    dx11_context()->Map(frame_transform_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &data, sizeof(FrameTransform));
    dx11_context()->Unmap(frame_transform_buffer_, 0);
}

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
    auto vs_blob_result = dx11_compileShader(L"../src/game_editor_qt/shaders/uber_shader.hlsl", &vsBlob, "VSPosMain" , "vs_5_0");
    auto ps_blob_result = dx11_compileShader(L"../src/game_editor_qt/shaders/uber_shader.hlsl", &psBlob, "PSColorOnly" , "ps_5_0");
    if (!vs_blob_result || !ps_blob_result) {
        exit(12344);
    }
    dx11_createVertexShaderFromByteCode(vsBlob, &uber_pos_vertex_shader_);
    //uber_vertex_shader_  = dx11_createVertexShaderFromByteCode(vsBlob);
    uber_pixel_shader_ = dx11_createPixelShaderFromByteCode(psBlob);
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    dx11_createInputLayout(layoutDesc, 1, vsBlob, &uber_input_layout_);

    {

        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(FrameTransform);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        HRESULT hr = dx11_device()->CreateBuffer(&desc, nullptr, &frame_transform_buffer_);
        if (FAILED(hr)) {
            printf("Failed to create object buffer\n");
            exit(1234);
        }
    }

    uploadGridData();

}

/**
 *
 * @param paint_event This gets called by windows on WM_PAINT, so we need to make sure
 * our 3d viewport is always showing the correct information according to the current
 * GameObject tree ("scenegraph").
 * For now we are just clearing and rendering a basic grid.
 * TODO access the (single) game-object-tree (the world), this is what we actually need to render.
 */
void D3DViewPortWidget::paintEvent(QPaintEvent *paint_event) {
    dx11_clearBackbuffer({0.1, 0.1, 0.2, 1});

    // The actual rendering of our 3d contents go here!
    // For now just render a grid!
    dx11_setShaderAndInputLayout(uber_input_layout_, uber_pos_vertex_shader_, uber_pixel_shader_);

    auto viewMatrix = glm::lookAtLH(glm::vec3{0, 4, 10}, {0, 0, 0}, {0,1, 0});
    auto projMatrix = glm::perspectiveLH(glm::radians(20.0f), 16/9.0f, 0.1f, 100.0f);
    updateWorldViewProjectionMatrix(glm::transpose(projMatrix * viewMatrix));
    dx11_setVertexShaderConstantBuffer(0, 1, frame_transform_buffer_);

    UINT stride = sizeof(glm::vec3);
    UINT offset = 0;

    {
        D3D11_VIEWPORT vp = {};
        vp.Width = width();
        vp.Height = height();
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        dx11_context()->RSSetViewports(1, &vp);
    }
    dx11_drawFromVertexBuffer(grid_vertex_buffer, stride, offset);

    dx11_presentBackbuffer();
}

QPaintEngine * D3DViewPortWidget::paintEngine() const {
    return nullptr; // disable Qt painting (important!)
}
