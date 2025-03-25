//
// Created by mgrus on 23.03.2025.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include <Windows.h>
#include <memory>
#include <string>

#include "RenderBackend.h"

class Widget;

class Application {
  public:
    Application(int w, int h, bool fullscreen);
    ~Application();

    void initialize(HINSTANCE hinstance, HINSTANCE h_prev_instance, LPSTR lpstr, int n_show_cmd);
    int run();

    bool changeResolution(int width, int height, int refreshRate, const std::string &deviceName, bool goFullscreen);

    RenderBackend* getRenderBackend() const;

    int scaled_width();
    int scaled_height();

protected:

    // Gets called right after the successful construction, must be implemented by concrete
    // application subclasses.
    virtual void onCreated() = 0;

    void setTopLevelWidget(const std::shared_ptr<Widget>& widget);

private:
    void mainLoop();
    static LRESULT CALLBACK AppWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    int height = 0;
    int width = 0;
    int fullscreen = 0;
    HWND _window;
    HDC hdc;
    std::shared_ptr<Widget> topLevelWidget;
    std::unique_ptr<RenderBackend> render_backend_;
    int scaled_width_ = -1;
    int scaled_height_ = -1;
};

std::shared_ptr<Application> getApplication();

#endif //APPLICATION_H
