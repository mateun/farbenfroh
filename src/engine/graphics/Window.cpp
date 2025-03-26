//
// Created by mgrus on 21.02.2025.
//

#include "Window.h"

#include <engine/graphics/MeshDrawData.h>


Window::Window(int w, int h, bool fullscreen) : width(w), height(h), fullscreen(fullscreen) {

}

Window::~Window() {
}

void Window::render() {
    MeshDrawData dd;
    //dd.mesh = getPlaneMesh();
}

// The winmain function
