//
// Created by mgrus on 23.04.2025.
//

#include <engine.h>
#include <glm/ext/matrix_transform.hpp>

std::vector<glm::vec3> getPositionData() {
    return std::vector<glm::vec3> {
        {-0.5, 0.5, 0},
        {-0.5, -0.5, 0},
        {0.5, -0.5, 0},
        {0.5, 0.5, 0},
    };
}

std::vector<uint32_t> getIndexData() {
    return std::vector<uint32_t> {
        0, 1, 2,
        0, 2, 3,
    };
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int) {

    auto win = create_window(800, 600, false, GetModuleHandle(NULL));

    initOpenGL46(win);
    HDC hdc = GetDC(win);
    renderer::setClearColor(0.5, 0, 0, 1);
    auto vsrc = renderer::vertexShaderBuilder()->position().mvp().build();
    auto fsrc = renderer::fragmentShaderBuilder()->color().build();
    auto vertexShader = renderer::compileVertexShader(vsrc);
    auto fragmentShader = renderer::compileFragmentShader(fsrc);
    auto myprog = renderer::linkShaderProgram(vertexShader, fragmentShader);
    auto worldMat = glm::translate(glm::mat4(1), glm::vec3(-1, 0, 0));
    auto result = renderer::setShaderValue(myprog, "mvpMatrix", worldMat);
    if (!result) {
        exit(999);
    }
    auto vbo = renderer::vertexBufferBuilder()->attributeVec3(renderer::VertexAttributeSemantic::Position, getPositionData()).build();
    std::vector<renderer::VertexAttribute> vertexAttributes = {
        renderer::VertexAttribute{
            .semantic = renderer::VertexAttributeSemantic::Position,
            .format = renderer::VertexAttributeFormat::Float3,
            .location = 0,
            .offset = 0,
            .components = 3,
            .stride = 12
        }

    };
    auto ibo = renderer::createIndexBuffer(getIndexData());
    auto triMesh = renderer::createMesh(vbo, ibo, vertexAttributes, getIndexData().size());

    bool run = true;
    while (run) {
        run = poll_window(win);
        renderer::clear();
        renderer::bindProgram(myprog);
        renderer::drawMesh(triMesh);
        renderer::present(hdc);


    }

}
