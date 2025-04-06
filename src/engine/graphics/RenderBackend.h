//
// Created by mgrus on 24.03.2025.
//

#ifndef RENDERBACKEND_H
#define RENDERBACKEND_H
#include <Windows.h>

#include "Widget.h"

class Camera;

enum class RenderBackendType {
    OpenGL,
    DX11,
    Vulkan,
    DX12,
    Software,
};


class RenderBackend {
public:
    RenderBackend(RenderBackendType type, HDC hdc, HWND hwnd, int width, int height);
    void setViewport(int x, int y, int width, int height);

    std::shared_ptr<Shader> getWidgetDefaultShader(bool textured = true) const;
    std::shared_ptr<Shader> getWidgetRoundedRectShader(bool textured) const;
    std::shared_ptr<Shader> getWidgetDefaultTextShader() const;

    std::shared_ptr<Camera> getOrthoCameraForViewport(int origin_x, int origin_y, float x, float y);



private:
    void initOpenGL();


    HDC hdc_;
    HWND hwnd_;
    int width_;
    int height_;
    RenderBackendType type_;

    std::shared_ptr<Camera> ortho_camera_;
    std::shared_ptr<Shader> default_widget_textured_shader_;
    std::shared_ptr<Shader> default_widget_colored_shader_;
    std::shared_ptr<Shader> default_text_shader_;
    std::shared_ptr<Shader> default_rounded_rect_shader_;
};



#endif //RENDERBACKEND_H
