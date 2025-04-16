//
// Created by mgrus on 16.04.2025.
//

#ifndef D3DVIEWPORTWIDGET_H
#define D3DVIEWPORTWIDGET_H

#include <complex.h>
#include <d3d11.h>
#include <QWidget>


class D3DViewPortWidget : public QWidget {

public:
    void uploadGridData();

    D3DViewPortWidget(QWidget* parent = nullptr);

    void paintEvent(QPaintEvent*) override;

    QPaintEngine* paintEngine() const override;

private:
    ID3D11Buffer * grid_vertex_buffer;
    ID3D11VertexShader* uber_vertex_shader_ = nullptr;
    ID3D11PixelShader* uber_pixel_shader_ = nullptr;
    ID3D11InputLayout* uber_input_layout_ = nullptr;

};



#endif //D3DVIEWPORTWIDGET_H
