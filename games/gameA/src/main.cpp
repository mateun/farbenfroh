//
// Created by mgrus on 23.04.2025.
//

#include <engine.h>
#include <glm/ext/matrix_clip_space.hpp>
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
    auto myprog = linkShaderProgram(vertexShader, fragmentShader);
    return myprog;
}

renderer::ProgramHandle createTextShader() {
    auto vsrc = renderer::vertexShaderBuilder()->position(0).uv(1).mvp().build();
    auto fsrc = renderer::fragmentShaderBuilder()->diffuseTexture(0).textRender().build();
    auto vertexShader = renderer::compileVertexShader(vsrc);
    auto fragmentShader = renderer::compileFragmentShader(fsrc);
    auto myprog = linkShaderProgram(vertexShader, fragmentShader);
    return myprog;
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int) {

    auto win = create_window(800, 600, false, GetModuleHandle(NULL));

    initOpenGL46(win, false, 4);
    HDC hdc = GetDC(win);
    renderer::setClearColor(0.5, 0, 0, 1);

    auto colorShader = createColorShader();
    auto textureShader = createTextureShader();
    auto textShader = createTextShader();

    auto worldMat = glm::translate(glm::mat4(1), glm::vec3(-1, 0, 0));
    auto projMat = glm::ortho<float>(0, 800, 0, 600.0f, 0.1f, 100.0f);
    auto result = renderer::setShaderValue(colorShader, "mvpMatrix", projMat * worldMat);

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

    // Font
    auto font = renderer::createFontFromFile("../../v2025/assets/consola.ttf", 14.0f);
    auto quadText = renderer::drawTextIntoQuad(font, "hello Is this looking correct now, or not?!:;");
    auto quadTextCapital = renderer::drawTextIntoQuad(font, "HELLO!g");

    // Postprocessing
    // Create our fullscreen render-target
    auto fullScreenRT = renderer::renderTargetBuilder()->size(800, 600).color().depth().build();


    bool run = true;
    while (run) {
        using namespace renderer;
        run = poll_window(win);
        setClearColor(0.5, 0.5, 0, 1);
        clear();
        bindRenderTarget(fullScreenRT);
        setClearColor(0, 0.5, 0, 1);
        clear();

        bindProgram(colorShader);
        drawMesh(quadMesh);

        bindProgram(textureShader);
        auto scaleMat = glm::scale(glm::mat4(1), glm::vec3(64, 64, 1));
        worldMat = glm::translate(worldMat, glm::vec3(100, 200, -0.1));
        setShaderValue(textureShader, "mvpMatrix", projMat * worldMat * scaleMat);
        bindTexture(porkImageTexture);
        drawMesh(quadMesh);


        worldMat = glm::translate(glm::mat4(1), glm::vec3(400, 300, -0.1));
        setShaderValue(textShader, "mvpMatrix", projMat * worldMat);
        bindTexture(font.atlasTexture);
        drawMesh(quadText);

        worldMat = glm::translate(glm::mat4(1), glm::vec3(480, 400, -0.1));
        setShaderValue(textShader, "mvpMatrix", projMat * worldMat);
        bindTexture(font.atlasTexture);
        drawMesh(quadTextCapital);



        bindDefaultRenderTarget();
        bindTexture(fullScreenRT.colorTex);
        scaleMat = scale(glm::mat4(1), glm::vec3(800, 600, 1));
        auto translateMat = glm::translate(glm::mat4(1), glm::vec3(400, 300, -0.1));
        setShaderValue(textureShader, "mvpMatrix", projMat * translateMat * scaleMat);
        drawMesh(quadMesh);
        present(hdc);


    }

}
