//
// Created by mgrus on 16.04.2025.
//

#include "D3DViewPortWidget.h"

#include <engine/dx11/dx11_api.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

void D3DViewPortWidget::createPseudoGlowBlendState() {
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    dx11_device()->CreateBlendState(&blendDesc, &additive_glow_blend_);
}

void D3DViewPortWidget::updateMaterialColor(glm::vec4 color) {
    MaterialBuffer data;
    data.baseColor = color;
    D3D11_MAPPED_SUBRESOURCE mapped;
    dx11_context()->Map(frame_material_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &data, sizeof(MaterialBuffer));
    dx11_context()->Unmap(frame_material_buffer_, 0);
    dx11_setPixelShaderConstantBuffer(1, 1, frame_material_buffer_);

}

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
    dx11_setVertexShaderConstantBuffer(0, 1, frame_transform_buffer_);
}

void D3DViewPortWidget::createFrameConstantBuffer(ID3D11Buffer** targetBuffer) {

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(FrameTransform);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    HRESULT hr = dx11_device()->CreateBuffer(&desc, nullptr, targetBuffer);
    if (FAILED(hr)) {
        printf("Failed to create object buffer\n");
        exit(1234);
    }

}

void D3DViewPortWidget::generateThickLineQuad(
    const glm::vec3& p0,
    const glm::vec3& p1,
    float thickness,
    std::vector<glm::vec3>& outVertices,
    std::vector<uint32_t>& outIndices)
{
    glm::vec3 dir = glm::normalize(p1 - p0);
    glm::vec3 up = glm::vec3(0, 1, 0); // assumes lines lie on XZ plane
    glm::vec3 right = glm::normalize(glm::cross(dir, up)) * (thickness * 0.5f);

    glm::vec3 v0 = p0 + right;
    glm::vec3 v1 = p0 - right;
    glm::vec3 v2 = p1 + right;
    glm::vec3 v3 = p1 - right;

    uint32_t baseIndex = static_cast<uint32_t>(outVertices.size());

    outVertices.push_back({ v0 });
    outVertices.push_back({ v1 });
    outVertices.push_back({ v2 });
    outVertices.push_back({ v3 });

    // Two triangles
    outIndices.push_back(baseIndex + 0);
    outIndices.push_back(baseIndex + 1);
    outIndices.push_back(baseIndex + 2);

    outIndices.push_back(baseIndex + 2);
    outIndices.push_back(baseIndex + 1);
    outIndices.push_back(baseIndex + 3);
}



void D3DViewPortWidget::uploadGridData() {

    int halfSize = 10;

    for (int i = -halfSize; i <= halfSize; ++i) {
        // Vertical lines (Z-axis)
        glm::vec3 p0(i, 0, -10);
        glm::vec3 p1(i, 0,  10);
        generateThickLineQuad(p0, p1, 0.05f, grid_verts_, grid_indices_);

        // Horizontal lines (X-axis)
        glm::vec3 p2(-10, 0, i);
        glm::vec3 p3( 10, 0, i);
        generateThickLineQuad(p2, p3, 0.05f, grid_verts_, grid_indices_);
    }

    grid_vertex_buffer_ = dx11_createVertexBuffer(grid_verts_);
    grid_index_buffer_ = dx11_createIndexBuffer(grid_indices_);
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

    uber_pos_vertex_shader_  = dx11_createVertexShaderFromByteCode(vsBlob);
    uber_pixel_shader_ = dx11_createPixelShaderFromByteCode(psBlob);
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    dx11_createInputLayout(layoutDesc, 1, vsBlob, &uber_input_layout_);
    createFrameConstantBuffer(&frame_transform_buffer_);
    createFrameConstantBuffer(&frame_material_buffer_);
    uploadGridData();

}

void D3DViewPortWidget::updateViewport() {
    D3D11_VIEWPORT vp = {};
    vp.Width = width();
    vp.Height = height();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    dx11_context()->RSSetViewports(1, &vp);
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
    // For now just render a grid - but with a slight glow effect :)

    // Common data for each pass:
    dx11_setShaderAndInputLayout(uber_input_layout_, uber_pos_vertex_shader_, uber_pixel_shader_);
    updateViewport();
    float aspect_ratio = width() / (float)height();
    auto viewMatrix = glm::lookAtLH(glm::vec3{0, 4, 10}, {0, 0, 0}, {0,1, 0});
    auto projMatrix = glm::perspectiveLH(glm::radians(20.0f), aspect_ratio, 0.1f, 100.0f);
    float blendFactor[4] = {0, 0, 0, 0};
    UINT stride = sizeof(glm::vec3);
    //dx11_context()->OMSetDepthStencilState(nullptr, 0);

    // Pass specific:
    // Halo Pass
    glm::mat4 haloTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.99f));
    glm::mat4 haloTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.01f, 0.0f));
    updateWorldViewProjectionMatrix(glm::transpose(projMatrix * viewMatrix * haloTranslation));
    updateMaterialColor({0.05, 0.05, 0.3, .15});
    dx11_context()->OMSetBlendState(additive_glow_blend_, blendFactor, 0xffffffff);
    //dx11_drawFromVertexBuffer(grid_vertex_buffer_, sizeof(glm::vec3), 0);
    dx11_drawFromIndexBuffer(grid_index_buffer_, grid_vertex_buffer_, stride, grid_indices_.size(), 0, 0);

    // Sharp line pass
    updateWorldViewProjectionMatrix(glm::transpose(projMatrix * viewMatrix * haloTransform));
    updateMaterialColor({1, .2, 1, .5});
    dx11_context()->OMSetBlendState(additive_glow_blend_, blendFactor, 0xffffffff);
    dx11_drawFromIndexBuffer(grid_index_buffer_, grid_vertex_buffer_, stride, grid_indices_.size(), 0, 0);

    dx11_presentBackbuffer();
}

QPaintEngine * D3DViewPortWidget::paintEngine() const {
    return nullptr; // disable Qt painting (important!)
}
