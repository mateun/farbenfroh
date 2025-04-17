//
// Created by mgrus on 16.04.2025.
//

#include "D3DViewPortWidget.h"

#include <iostream>
#include <QResizeEvent>
#include <engine/dx11/dx11_api.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

/**
 *
 * @param sceneData Provided the latest version of sceneData,
 * so the viewport can render it.
 */
void D3DViewPortWindow::onSceneUpdate(SceneData* sceneData) {

}

void D3DViewPortWindow::createPseudoGlowBlendState() {
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

void D3DViewPortWindow::updateMaterialColor(glm::vec4 color) {
    MaterialBuffer data;
    data.baseColor = color;
    D3D11_MAPPED_SUBRESOURCE mapped;
    auto hr = dx11_context()->Map(frame_material_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr)) {
        exit(999);
    }
    memcpy(mapped.pData, &data, sizeof(MaterialBuffer));
    dx11_context()->Unmap(frame_material_buffer_, 0);
    dx11_setPixelShaderConstantBuffer(1, 1, frame_material_buffer_);

}

/**
 * Used to update the main transform matrix for every object, every frame.
 * @param worldViewProj The updated, new main transform matrix
 */
void D3DViewPortWindow::updateWorldViewProjectionMatrix(const glm::mat4& worldViewProj) {
    FrameTransform data;
    data.worldViewProj = worldViewProj;

    D3D11_MAPPED_SUBRESOURCE mapped;
    dx11_context()->Map(frame_transform_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &data, sizeof(FrameTransform));
    dx11_context()->Unmap(frame_transform_buffer_, 0);
    dx11_setVertexShaderConstantBuffer(0, 1, frame_transform_buffer_);
}

void D3DViewPortWindow::createFrameConstantBuffer(ID3D11Buffer** targetBuffer) {

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

void D3DViewPortWindow::generateThickLineQuad(
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



void D3DViewPortWindow::uploadGridData() {

    int halfSize = 10;

    for (int i = -halfSize; i <= halfSize; ++i) {
        // Vertical lines (Z-axis)
        glm::vec3 p0(i, 0, -10);
        glm::vec3 p1(i, 0,  10);
        generateThickLineQuad(p0, p1, 0.01f, grid_verts_, grid_indices_);

        // Horizontal lines (X-axis)
        glm::vec3 p2(-10, 0, i);
        glm::vec3 p3( 10, 0, i);
        generateThickLineQuad(p2, p3, 0.01f, grid_verts_, grid_indices_);
    }

    grid_vertex_buffer_ = dx11_createVertexBuffer(grid_verts_);
    grid_index_buffer_ = dx11_createIndexBuffer(grid_indices_);
}

void D3DViewPortWindow::exposeEvent(QExposeEvent*) {
    if (isExposed()) {
        static bool firstTime = true;
        if (firstTime) {
            dx11_init(reinterpret_cast<HWND>(winId())); // pass HWND once
            firstTime = false;

                ID3DBlob* vsBlob;
                ID3DBlob* psBlob;
                auto vs_blob_result = dx11_compileShader(L"../src/game_editor_qt/shaders/uber_shader.hlsl", &vsBlob, "VSPosMain" , "vs_5_0");
                auto ps_blob_result = dx11_compileShader(L"../src/game_editor_qt/shaders/uber_shader.hlsl", &psBlob, "PSColorOnly" , "ps_5_0");
                if (!vs_blob_result || !ps_blob_result) {
                    exit(12344);
                }

                uber_pos_vertex_shader_  = dx11_createVertexShaderFromByteCode(vsBlob);
                uber_color_only_pixel_shader_ = dx11_createPixelShaderFromByteCode(psBlob);
                D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
                    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                };

                dx11_createInputLayout(layoutDesc, 1, vsBlob, &uber_input_layout_);
                createFrameConstantBuffer(&frame_transform_buffer_);
                createFrameConstantBuffer(&frame_material_buffer_);
                uploadGridData();

                cube_mesh_ = new dx11_Mesh();   // empty create
                dx11_loadFirstMeshFromFile("../assets/blocky_guy.glb", cube_mesh_);

                // Textured shader
                vs_blob_result = dx11_compileShader(L"../src/game_editor_qt/shaders/uber_shader.hlsl", &vsBlob, "VSMain" , "vs_5_0");
                ps_blob_result = dx11_compileShader(L"../src/game_editor_qt/shaders/uber_shader.hlsl", &psBlob, "PSMain" , "ps_5_0");
                if (!vs_blob_result || !ps_blob_result) {
                    exit(12344);
                }

                // Setup textured + normal shaders and input layout
                uber_pos_uv_vertex_shader_  = dx11_createVertexShaderFromByteCode(vsBlob);
                uber_textured_pixel_shader_ = dx11_createPixelShaderFromByteCode(psBlob);
                D3D11_INPUT_ELEMENT_DESC layout_pos_uv_normal[] = {
                    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                          D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, sizeof(float) * 3,          D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * (3 + 2),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
                };

                dx11_createInputLayout(layout_pos_uv_normal, 3, vsBlob, &uber_pos_uv_normal_input_layout_);

                // Texture loading
                blocky_guy_texture_ = dx11_loadTextureFromFile("../assets/blocky_guy_diffuse.png");
                blocky_guy_srv_ = dx11_createShaderResourceView(blocky_guy_texture_);
                sampler_state_ = dx11_createSamplerState();
        }
        else {
            render();
        }

    }
}

D3DViewPortWindow::D3DViewPortWindow(QWidget* parent) {
    setSurfaceType(QSurface::RasterSurface);
    // HWND hwnd = reinterpret_cast<HWND>(winId());
    // dx11_init(hwnd);



}

void D3DViewPortWindow::updateViewport() {
    if (!dx11_context()) return;

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
void D3DViewPortWindow::render() {
    dx11_clearBackbuffer({0.1, 0.1, 0.2, 1});

    // The actual rendering of our 3d contents go here!
    // For now just render a grid - but with a slight glow effect :)

    // Common data for each pass:
    dx11_setShaderAndInputLayout(uber_input_layout_, uber_pos_vertex_shader_, uber_color_only_pixel_shader_);
    updateViewport();


    auto viewMatrix = glm::lookAtLH(glm::vec3{0, 10, 11}, {0, 0, 0}, {0,1, 0});

    float aspect_ratio = (float)width() / (float)height();
    auto projMatrix = glm::perspectiveLH(glm::radians(20.0f), aspect_ratio, 0.1f, 100.0f);

    float blendFactor[4] = {0, 0, 0, 0};
    UINT stride = sizeof(glm::vec3);
    //dx11_context()->OMSetDepthStencilState(nullptr, 0);

    // Pass specific:
    // Halo Pass
    glm::mat4 haloTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.99f));
    glm::mat4 haloTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.1f, 0.0f));
    updateWorldViewProjectionMatrix(glm::transpose(projMatrix * viewMatrix * haloTranslation));
    updateMaterialColor({0.05, 0.05, 0.3, .15});
    dx11_context()->OMSetBlendState(additive_glow_blend_, blendFactor, 0xffffffff);
    //dx11_drawFromVertexBuffer(grid_vertex_buffer_, sizeof(glm::vec3), 0);
    //dx11_drawFromIndexBuffer(grid_index_buffer_, grid_vertex_buffer_, stride, grid_indices_.size(), 0, 0);

    // Sharp line pass
    updateWorldViewProjectionMatrix(glm::transpose(projMatrix * viewMatrix));
    updateMaterialColor({1, .2, 1, .5});
    dx11_context()->OMSetBlendState(additive_glow_blend_, blendFactor, 0xffffffff);
    dx11_drawFromIndexBuffer(grid_index_buffer_, grid_vertex_buffer_, stride, grid_indices_.size(), 0, 0);


    // Now draw our mesh!
    dx11_setShaderAndInputLayout(uber_pos_uv_normal_input_layout_, uber_pos_uv_vertex_shader_, uber_textured_pixel_shader_);
    dx11_context()->PSSetShaderResources(0, 1, &blocky_guy_srv_);
    dx11_context()->PSSetSamplers(0, 1, &sampler_state_);
    stride = sizeof(float) * 8; // 3 pos + 2 uv + 3 normal
    updateMaterialColor({1, 0, 0, 1});

    dx11_drawFromIndexBuffer(cube_mesh_->indexBuffer, cube_mesh_->pos_uv_normal_buffer, stride, cube_mesh_->indices.size(), 0, 0);


    dx11_presentBackbuffer();
}

void D3DViewPortWindow::resizeEvent(QResizeEvent* event) {
    printf("QWindow size: %d, %d" ,width(), height());
    RECT r;
    GetClientRect((HWND)winId(), &r);
    printf("HWND client size: %d %d", r.right - r.left , r.bottom - r.top);

    dx11_onResize(event->size().width(), event->size().height());
    updateViewport();


}



// QPaintEngine * D3DViewPortWidget::paintEngine() const {
//     return nullptr; // disable Qt painting (important!)
// }
