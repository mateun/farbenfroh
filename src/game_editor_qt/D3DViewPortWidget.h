//
// Created by mgrus on 16.04.2025.
//

#ifndef D3DVIEWPORTWIDGET_H
#define D3DVIEWPORTWIDGET_H

#include <complex.h>
#include <d3d11.h>
#include <QWidget>
#include <QWindow>
#include <glm/glm.hpp>


struct dx11_Mesh;
/**
 * This is the API which the 3d-viewport offers to
 * render scene objects.
 *
 */
struct SceneData {

};

struct MaterialBuffer {
    glm::vec4 baseColor;
};

struct FrameTransform {
    glm::mat4 worldViewProj;
};

class D3DViewPortWindow : public QWindow {
public:

    D3DViewPortWindow(QWidget *parent = nullptr);

    void uploadGridData();

    void exposeEvent(QExposeEvent*) override;

    void generateThickLineQuad(
        const glm::vec3 &p0,
        const glm::vec3 &p1,
        float thickness,
        std::vector<glm::vec3> &outVertices,
        std::vector<uint32_t> &outIndices);



    void updateViewport();

    void render();

    void resizeEvent(QResizeEvent *event) override;

    void onSceneUpdate(SceneData *sceneData);

    void createPseudoGlowBlendState();

    void updateWorldViewProjectionMatrix(const glm::mat4 &worldViewProj);

    void updateMaterialColor(glm::vec4 color);

    void createFrameConstantBuffer(ID3D11Buffer **targetBuffer);

    //QPaintEngine *paintEngine() const override;

private:
    ID3D11Buffer *grid_vertex_buffer_ = nullptr;
    ID3D11Buffer *grid_index_buffer_ = nullptr;
    ID3D11SamplerState * sampler_state_ = nullptr;
    ID3D11Texture2D * blocky_guy_texture_ = nullptr;
    ID3D11ShaderResourceView * blocky_guy_srv_ = nullptr;
    ID3D11VertexShader *uber_pos_uv_vertex_shader_ = nullptr;
    ID3D11VertexShader *uber_pos_vertex_shader_ = nullptr;
    ID3D11PixelShader *uber_color_only_pixel_shader_ = nullptr;
    ID3D11PixelShader *uber_textured_pixel_shader_ = nullptr;
    ID3D11InputLayout *uber_input_layout_ = nullptr;
    ID3D11InputLayout * uber_pos_uv_normal_input_layout_ = nullptr;
    ID3D11Buffer *frame_transform_buffer_ = nullptr;
    ID3D11Buffer *frame_material_buffer_ = nullptr;
    ID3D11BlendState *additive_glow_blend_ = nullptr;

    dx11_Mesh* cube_mesh_ = nullptr;
    std::vector<glm::vec3> grid_verts_;
    std::vector<uint32_t> grid_indices_;

    float v_fov_original_ = 0;
    float h_fov_original_ = 0;

};


#endif //D3DVIEWPORTWIDGET_H
