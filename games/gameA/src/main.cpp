//
// Created by mgrus on 23.04.2025.
//

#include <engine.h>

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int) {

    auto win = create_window(800, 600, false, GetModuleHandle(NULL));

    initOpenGL46(win);
    HDC hdc = GetDC(win);
    renderer::setClearColor(0.5, 0, 0, 1);
    auto vsrc = renderer::vertexShaderBuilder()->position().build();
    auto fsrc = renderer::fragmentShaderBuilder()->color().build();
    auto vertexShader = renderer::compileVertexShader(vsrc);
    auto fragmentShader = renderer::compileFragmentShader(fsrc);
    auto myprog = renderer::linkShaderProgram(vertexShader, fragmentShader);

    bool run = true;
    while (run) {
        renderer::clear();
        run = poll_window(win);
        renderer::present(hdc);


    }

}
