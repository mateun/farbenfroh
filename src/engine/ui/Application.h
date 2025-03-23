//
// Created by mgrus on 23.03.2025.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include <Windows.h>
#include <memory>

class Application {
  public:
    Application(int w, int h, bool fullscreen);
    ~Application();

    void initialize(HINSTANCE hinstance, HINSTANCE h_prev_instance, LPSTR lpstr, int n_show_cmd);
    int run();

private:
    void mainLoop();
    static LRESULT CALLBACK AppWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    int height = 0;
    int width = 0;
    int fullscreen = 0;
    HWND _window;
};



#endif //APPLICATION_H
