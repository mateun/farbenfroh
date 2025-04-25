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

std::vector<glm::vec2> getUVData() {
    return std::vector<glm::vec2> {
        {0, 0},
        {0, 1},
        {1, 1},
        {1, 0}
    };
}

std::vector<uint32_t> getIndexData() {
    return std::vector<uint32_t> {
        0, 1, 2,
        0, 2, 3,
    };
}

renderer::ProgramHandle createColorShader() {
    auto vsrc = renderer::vertexShaderBuilder()->position(0).mvp().build();
    auto fsrc = renderer::fragmentShaderBuilder()->color().build();
    auto vertexShader = renderer::compileVertexShader(vsrc);
    auto fragmentShader = renderer::compileFragmentShader(fsrc);
    auto myprog = renderer::linkShaderProgram(vertexShader, fragmentShader);
    return myprog;
}

renderer::ProgramHandle createTextureShader() {
    auto vsrc = renderer::vertexShaderBuilder()->position(0).uv(1).mvp().build();
    auto fsrc = renderer::fragmentShaderBuilder()->diffuseTexture(0).build();
    auto vertexShader = renderer::compileVertexShader(vsrc);
    auto fragmentShader = renderer::compileFragmentShader(fsrc);
    auto myprog = renderer::linkShaderProgram(vertexShader, fragmentShader);
    return myprog;
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int) {

    auto win = create_window(800, 600, false, GetModuleHandle(NULL));

    initOpenGL46(win, false, 4);
    HDC hdc = GetDC(win);
    renderer::setClearColor(0.5, 0, 0, 1);

    auto colorShader = createColorShader();
    auto textureShader = createTextureShader();

    auto worldMat = glm::translate(glm::mat4(1), glm::vec3(-1, 0, 0));
    auto result = renderer::setShaderValue(colorShader, "mvpMatrix", worldMat);

    auto vbo = renderer::vertexBufferBuilder()->
            attributeVec3(renderer::VertexAttributeSemantic::Position, getPositionData()).
            attributeVec2(renderer::VertexAttributeSemantic::UV0, getUVData()).
            build();

    // TODO get the automatically calculated vertexAttributes out of the vertexBufferBuilder,
    // so we don't need to manually define this data here again. Error-prone and tedious.
    // OTOH the collection of the attributes currently has a bug with accumulating the current stride.
    // This would need to be fixed first, e.g. post-process on-build set the end-stride to all attributes.
    std::vector<renderer::VertexAttribute> vertexAttributes = {
        renderer::VertexAttribute{
            .semantic = renderer::VertexAttributeSemantic::Position,
            .format = renderer::VertexAttributeFormat::Float3,
            .location = 0,
            .offset = 0,
            .components = 3,
            .stride = 20
        },
         renderer::VertexAttribute{
            .semantic = renderer::VertexAttributeSemantic::UV0,
            .format = renderer::VertexAttributeFormat::Float2,
            .location = 1,
            .offset = 12,
            .components = 2,
            .stride = 20
        }

    };
    auto ibo = renderer::createIndexBuffer(getIndexData());
    auto quadMesh = renderer::createMesh(vbo, ibo, vertexAttributes, getIndexData().size());

    // Texturing
    auto mainBackground = renderer::createImageFromFile("../../games/gameA/assets/captain_pork.png");
    auto porkImageTexture = renderer::createTexture(mainBackground);

    // Postprocessing
    // Create our fullscreen render-target
    auto fullScreenRT = renderer::renderTargetBuilder()->size(800, 600).color().depth().build();

    bool run = true;
    while (run) {
        run = poll_window(win);
        renderer::setClearColor(0.5, 0.5, 0, 1);
        renderer::clear();
        renderer::bindRenderTarget(fullScreenRT);
        renderer::setClearColor(0, 0.5, 0, 1);
        renderer::clear();

        renderer::bindProgram(colorShader);
        renderer::drawMesh(quadMesh);

        renderer::bindProgram(textureShader);
        worldMat = glm::translate(glm::mat4(1), glm::vec3(1, 0.25, 0));
        result = renderer::setShaderValue(textureShader, "mvpMatrix", worldMat);
        renderer::bindTexture(porkImageTexture);
        renderer::drawMesh(quadMesh);

        renderer::bindDefaultRenderTarget();
        renderer::bindTexture(fullScreenRT.colorTex);
        auto scaleMat = glm::scale(glm::mat4(1), glm::vec3(2, 2, 1));
        auto translateMat = glm::translate(glm::mat4(1), glm::vec3(0, 0, -0.1));
        result = renderer::setShaderValue(textureShader, "mvpMatrix", translateMat * scaleMat);
        renderer::drawMesh(quadMesh);

        renderer::present(hdc);


    }

}
