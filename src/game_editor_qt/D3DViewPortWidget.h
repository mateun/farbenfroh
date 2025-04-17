//
// Created by mgrus on 16.04.2025.
//

#ifndef D3DVIEWPORTWIDGET_H
#define D3DVIEWPORTWIDGET_H

#include <complex.h>
#include <d3d11.h>
#include <QWidget>
#include <glm/glm.hpp>

struct MaterialBuffer {
    glm::vec4 baseColor;
    bool useTexture = false;
    glm::vec3 padding;
};

struct FrameTransform {
    glm::mat4 worldViewProj;
};

class D3DViewPortWidget : public QWidget {
public:
    void uploadGridData();

    void generateThickLineQuad(
        const glm::vec3 &p0,
        const glm::vec3 &p1,
        float thickness,
        std::vector<glm::vec3> &outVertices,
        std::vector<uint32_t> &outIndices);

    D3DViewPortWidget(QWidget *parent = nullptr);

    void updateViewport();

    void paintEvent(QPaintEvent *) override;

    void createPseudoGlowBlendState();

    void updateWorldViewProjectionMatrix(const glm::mat4 &worldViewProj);

    void updateMaterialColor(glm::vec4 color);

    void createFrameConstantBuffer(ID3D11Buffer **targetBuffer);

    QPaintEngine *paintEngine() const override;

private:
    ID3D11Buffer *grid_vertex_buffer_ = nullptr;
    ID3D11Buffer *grid_index_buffer_ = nullptr;
    ID3D11VertexShader *uber_pos_uv_vertex_shader_ = nullptr;
    ID3D11VertexShader *uber_pos_vertex_shader_ = nullptr;
    ID3D11PixelShader *uber_pixel_shader_ = nullptr;
    ID3D11InputLayout *uber_input_layout_ = nullptr;
    ID3D11Buffer *frame_transform_buffer_ = nullptr;
    ID3D11Buffer *frame_material_buffer_ = nullptr;
    ID3D11BlendState *additive_glow_blend_ = nullptr;

    std::vector<glm::vec3> grid_verts_;
    std::vector<uint32_t> grid_indices_;
};


#endif //D3DVIEWPORTWIDGET_H
