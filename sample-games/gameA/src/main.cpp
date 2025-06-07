//
// Created by mgrus on 23.04.2025.
//

#include <engine.h>
#include <iostream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <lua_parser.h>
#include <lua.hpp>
#include <GL/glew.h>
#include <opengl46.h>
#include <lauxlib.h>

extern int mouse_x;
extern int mouse_y;

std::vector<glm::vec3> getPositionData() {
    return std::vector<glm::vec3> {
        // {0, 1, 0},
        // {0, 0, 0},
        // {1, 0, 0},
        // {1, 1, 0},
            {0, 0, 0},
            {0, 1, 0},
            {1, 1, 0},
            {1, 0, 0},
    };
}

std::vector<glm::vec2> getUVData() {
    return std::vector<glm::vec2> {
        {0, 0},
        {0, 1},
        {1, 1},
        {1, 0}
            // {0, 1},
            // {0, 0},
            // {1, 0},
            // {1, 1}
    };
}

renderer::IndexBufferDesc getIndexData() {
    renderer::IndexBufferDesc ibd;
    ibd.format = GL_UNSIGNED_INT;
    ibd.size_in_bytes = 6 * sizeof(uint32_t);
    return ibd;
}

renderer::ProgramHandle createColorShader() {
    auto vsrc = renderer::vertexShaderBuilder()->position(0).mvp().build();
    auto fsrc = renderer::fragmentShaderBuilder()->color().build();
    auto vertexShader = renderer::compileVertexShader(vsrc);
    auto fragmentShader = renderer::compileFragmentShader(fsrc);
    auto myprog = renderer::linkShaderProgram(vertexShader, fragmentShader);
    return myprog;
}

renderer::ProgramHandle createTiledTextureShader() {
    auto vsrc = renderer::vertexShaderBuilder()->position(0).uv(1).mvp().build();
    auto fsrc = renderer::fragmentShaderBuilder()->diffuseTexture(0, false).build();
    auto vertexShader = renderer::compileVertexShader(vsrc);
    auto fragmentShader = renderer::compileFragmentShader(fsrc);
    auto myprog = linkShaderProgram(vertexShader, fragmentShader);
    return myprog;
}

renderer::ProgramHandle createFullScreenQuadShader() {
    auto vsrc = renderer::vertexShaderBuilder()->position(0).uv(1).mvp().build();
    auto fsrc = renderer::fragmentShaderBuilder()->diffuseTexture(0, true).build();
    auto vertexShader = renderer::compileVertexShader(vsrc);
    auto fragmentShader = renderer::compileFragmentShader(fsrc);
    auto myprog = linkShaderProgram(vertexShader, fragmentShader);
    return myprog;
}

renderer::ProgramHandle createTextShader() {
    auto vsrc = renderer::vertexShaderBuilder()->position(0).uv(1).mvp().build();
    auto fsrc = renderer::fragmentShaderBuilder()->diffuseTexture(0, false).textRender().build();
    auto vertexShader = renderer::compileVertexShader(vsrc);
    auto fragmentShader = renderer::compileFragmentShader(fsrc);
    auto myprog = linkShaderProgram(vertexShader, fragmentShader);
    return myprog;
}

// To load a level, we parse a .lua file.
// This lua file has some top level elements for entities, ui etc.
// For each of these, a parser function exists:
void load_level(const std::string& filename) {
    std::string baseDir = "../../sample-games/gameA/levels";
    auto fileName = baseDir + "/level1.lua";

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    auto entities = load_entities(L, fileName.c_str());
    for (auto& e : entities) {
        std::cout << "id: " << e.id << std::endl;
        for (auto s : e.scripts) {
            std::cout << "\tscript: " << s << std::endl;
            execute_lua_file(L, baseDir + "/" + s);
        }

    }

    auto ui = load_ui(L, fileName.c_str());

    for (const auto& e : ui) {
        std::cout << "UI Element: " << e.id << ", Type: " << e.type
                  << ", Text: '" << e.text << "', Pos: (" << e.position[0]
                  << ", " << e.position[1] << ")" << std::endl;
    }


}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int) {

    auto win = create_window(800, 600, false, GetModuleHandle(NULL));

    initOpenGL46(win, true, 0);
    HDC hdc = GetDC(win);
    renderer::setClearColor(0.5, 0, 0, 1);

    auto colorShader = createColorShader();
    auto textureShader = createTiledTextureShader();
    auto textShader = createTextShader();
    auto fullScreenQuadShader = createFullScreenQuadShader();



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
    renderer::IndexBufferDesc ibd = getIndexData();
    auto quad_indices = std::vector<uint32_t> {
        0, 1, 2,
        0, 2, 3,
        // 2, 1, 0,
        // 3, 2, 0
    };
    ibd.data = quad_indices.data();
    auto ibo = renderer::createIndexBuffer(ibd);
    auto quadMesh = renderer::createMesh(vbo, ibo, vertexAttributes, 6);

    // Mesh loading
    auto loaded_mesh = renderer::importMesh("../../sample-games/gameA/assets/simple-geo.glb");
    auto animated_plane = renderer::importMesh("../../sample-games/gameA/assets/animated_plane.glb");

    // Texturing
    auto mainBackground = renderer::createImageFromFile("../../sample-games/gameA/assets/captain_pork2.png");
    auto porkImageTexture = createTexture(mainBackground);
    auto mainTitleImage = renderer::createImageFromFile("../../sample-games/gameA/assets/title_text3.png");
    auto mainTitleTexture = createTexture(mainTitleImage);
    auto btnPlayImage = renderer::createImageFromFile("../../sample-games/gameA/assets/btn_play_normal.png");
    auto btnPlayTexture = renderer::createTexture(btnPlayImage);
    auto btnExitImage = renderer::createImageFromFile("../../sample-games/gameA/assets/btn_exit_normal.png");
    auto btnExitTexture = renderer::createTexture(btnExitImage);

    // Font
    auto font = renderer::createFontFromFile("../../v2025/assets/consola.ttf", 14.0f);
    auto quadText = renderer::drawTextIntoQuad(font, "A!aBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz?!:;");
    auto quadTextCapital = renderer::drawTextIntoQuad(font, "FPS:");

    // Postprocessing
    // Create our fullscreen render-target
    auto fullScreenRT = renderer::renderTargetBuilder()->size(800, 600).color().depth().build();


    // Load our level with lua
    // load level
    load_level("../../sample-games/gameA/levels/level1.lua");


    bool run = true;
    while (run) {
        using namespace renderer;
        run = poll_window(win);
        setClearColor(0.0, 0.0, 0, 1);
        clear();
        bindRenderTarget(fullScreenRT);
        setClearColor(0.02, 0.01, 0.01, 1);
        clear();

        // Input tests
        if (keyPressed(VK_ESCAPE)) {
            run = false;
        }

        // Draw the single colored quad
        bindProgram(colorShader);
        auto scaleMat = glm::scale(glm::mat4(1), glm::vec3(32, 32, 1));
        auto worldMat = glm::translate(glm::mat4(1), glm::vec3(5, 10, 0));
        auto viewMat = glm::lookAt<float>(glm::vec3{0,0,5}, {0, 0, -3}, {0, 1, 0});
        auto projMat = glm::ortho<float>(0, 800, 600, 0.0f, .1f, 100.0f);
        setShaderValue(colorShader, "mvpMatrix", projMat * viewMat * worldMat * scaleMat);
        drawMesh(quadMesh, "singleColorQuad");

        scaleMat = glm::scale(glm::mat4(1), glm::vec3(1, 1, 1));
        worldMat = glm::translate(glm::mat4(1), glm::vec3(0, 1, -2));
        auto viewMatM = glm::lookAt<float>(glm::vec3{0,4,5}, {0, 0, -3}, {0, 1, 0});
        auto projMatPersp = glm::perspective<float>(glm::radians(40.0f), 16.0f/9.0f, 0.1, 500.0f);
        setShaderValue(colorShader, "mvpMatrix", projMatPersp * viewMatM * worldMat * scaleMat);
        drawMesh(loaded_mesh, "loadedModelMesh");

        scaleMat = glm::scale(glm::mat4(1), glm::vec3(.5, .5, 1));
        worldMat = glm::translate(glm::mat4(1), glm::vec3(-2, -1, -2));
        setShaderValue(colorShader, "mvpMatrix", projMatPersp * viewMatM * worldMat * scaleMat);
        drawMesh(animated_plane, "animatedPlane");

        // Title sprite
        bindProgram(textureShader);
        scaleMat = glm::scale(glm::mat4(1), glm::vec3(mainBackground.width/3, mainBackground.height/3, 1));
        worldMat = glm::translate(glm::mat4(1), glm::vec3(400 - (mainBackground.width/3/2), 0, -2));
        setShaderValue(textureShader, "mvpMatrix", projMat * viewMat * worldMat * scaleMat);
        bindTexture(porkImageTexture);
        drawMesh(quadMesh, "porkImage");

        // Title Image
        bindProgram(textureShader);
        scaleMat = glm::scale(glm::mat4(1), glm::vec3(mainTitleImage.width / 1.25, mainTitleImage.height / 1.25, 1));
        worldMat = glm::translate(glm::mat4(1), glm::vec3(800 * 0.125, 10, -1));
        setShaderValue(textureShader, "mvpMatrix", projMat * viewMat * worldMat * scaleMat);
        bindTexture(mainTitleTexture);
        drawMesh(quadMesh);

        // Button play
        bindProgram(textureShader);
        scaleMat = glm::scale(glm::mat4(1), glm::vec3(btnPlayImage.width/2, btnPlayImage.height/3, 1));
        worldMat = glm::translate(glm::mat4(1), glm::vec3(400 - (btnPlayImage.width/4), 460, -1));
        setShaderValue(textureShader, "mvpMatrix", projMat * viewMat * worldMat * scaleMat);
        bindTexture(btnPlayTexture);
        drawMesh(quadMesh);

        // Button exit
        bindProgram(textureShader);
        scaleMat = glm::scale(glm::mat4(1), glm::vec3(btnPlayImage.width/2, btnPlayImage.height/3, 1));
        worldMat = glm::translate(glm::mat4(1), glm::vec3(400 - (btnPlayImage.width/4), 510, -1));
        setShaderValue(textureShader, "mvpMatrix", projMat * viewMat * worldMat * scaleMat);
        bindTexture(btnExitTexture);
        drawMesh(quadMesh);



        worldMat = glm::translate(glm::mat4(1), glm::vec3(5, 300, -0.1));
        setShaderValue(textShader, "mvpMatrix", projMat * worldMat);
        bindTexture(font.atlasTexture);
        //drawMesh(quadText);

        worldMat = glm::translate(glm::mat4(1), glm::vec3(5, 600 - 8, -0.1));
        setShaderValue(textShader, "mvpMatrix", projMat * worldMat);
        bindTexture(font.atlasTexture);
        static int frame = 0;
        frame++;
        updateText(quadTextCapital, font, "Frame: " + std::to_string(frame));
        drawMesh(quadTextCapital);

        worldMat = glm::translate(glm::mat4(1), glm::vec3(200, 600 - 8, -0.1));
        setShaderValue(textShader, "mvpMatrix", projMat * worldMat);
        updateText(quadTextCapital, font, "mouse: " + std::to_string(mouseX()) + "/" + std::to_string(mouseY()));
        drawMesh(quadTextCapital);


        bindDefaultRenderTarget();
        bindTexture(fullScreenRT.colorTex);
        scaleMat = scale(glm::mat4(1), glm::vec3(800, 600, 1));
        auto translateMat = glm::translate(glm::mat4(1), glm::vec3(0, 0, -0.1));
        setShaderValue(fullScreenQuadShader, "mvpMatrix", projMat * translateMat * scaleMat);
        drawMesh(quadMesh);
        present(hdc);


    }

}
