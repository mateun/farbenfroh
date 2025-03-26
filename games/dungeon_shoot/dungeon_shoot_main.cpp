//
// Created by mgrus on 25.08.2024.
//

#include "dungeon_shoot.h"
#include "../src/engine/editor/editor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <lua.hpp>


void DungeonGame::init() {
    DefaultGame::init();

    getGameplayCamera()->updateLocation({0, 5, 20});
    getGameplayCamera()->updateLookupTarget({0, 5, -2});

    // Setup physics objects
    auto po1 = new PhysicsObject({0, 15, -3}, 20000, getMeshByName("house"));
    auto po2 = new PhysicsObject({-5, 15, -3}, 80, getMeshByName("hero"));
    _physicsObjects.push_back(po1);
    _physicsObjects.push_back(po2);

    editor = new Editor();


    // Lua stuff
    auto L =  luaL_newstate();
    luaL_openlibs(L);
    // Lua script to execute
    const char *lua_script = "x = 10; y = 20; z = x + y";

    // Load and run the Lua script
    if (luaL_dostring(L, lua_script) != LUA_OK) {
        fprintf(stderr, "Error executing Lua script: %s\n", lua_tostring(L, -1));
        lua_close(L);
    }


    // Push the variable 'z' from the Lua script onto the stack
    lua_getglobal(L, "z");

    // Check if 'z' is a number and print it
    if (lua_isnumber(L, -1)) {
        double z = lua_tonumber(L, -1);
        printf("Value of z from Lua: %.2f\n", z);
    } else {
        printf("'z' is not a number\n");
    }

    // Pop the 'z' variable off the stack
    lua_pop(L, 1);

    // Close the Lua state
    lua_close(L);
    // END LUA

}

void DungeonGame::renderPrimObjects() {

    // To find the nearest vertex in the scene to the mouse.
    // We compare the current mouse pos with every vertex
    // screen position of every object here.
    float shortestMouseVertexDistance = 1000;
    PrimObject* currentSelectedPrimObject = nullptr;

    for (auto po : _primObjects) {
        bindCamera(getGameplayCamera());
        lightingOn();
        location(po->pivotPosition);
        scale(po->scale);
        foregroundColor({1, 0, 0, 1});
        rotation(po->rotation);
        drawPlane();
        rotation({0, 0, 0});


        {
            // Render each vertex in screen space (UI camera)
            using namespace glm;
            glm::vec3 rotation = po->rotation;
            auto camera = getGameplayCamera();
            mat4 mattrans = translate(mat4(1), po->pivotPosition);
            mat4 matscale = scale(mat4(1), po->scale);
            mat4 matrotX = glm::rotate(mat4(1), glm::radians(rotation.x), {1, 0, 0});
            mat4 matrotY = glm::rotate(mat4(1), glm::radians(rotation.y), {0, 1, 0});
            mat4 matrotZ = glm::rotate(mat4(1), glm::radians(rotation.z), {0, 0, 1});
            mat4 matworld = mattrans * matrotX * matrotY * matrotZ * matscale;


            glm::vec2 ssTL = modelToScreenSpace({-0.5, 0.5, 0}, matworld, camera);
            glm::vec2 ssBL = modelToScreenSpace({-0.5, -0.5, 0}, matworld, camera);
            glm::vec2 ssBR = modelToScreenSpace({0.5, -0.5, 0}, matworld, camera);
            glm::vec2 ssTR = modelToScreenSpace({0.5, 0.5, 0}, matworld, camera);

            bindCamera(getUICamera());
            lightingOff();
            location({ssTL.x, ssTL.y, -0.7});
            scale({2, 2, 1});
            foregroundColor({0.4, 0.4, 0.4, 1});
            drawPlane();

            location({ssBL.x, ssBL.y, -0.7});
            drawPlane();
            location({ssBR.x, ssBR.y, -0.7});
            drawPlane();
            location({ssTR.x, ssTR.y, -0.7});
            drawPlane();


            // Handle the current vertex selection algo here, maybe move this out to update or similar:
            float dist = glm::abs(glm::distance(glm::vec2(mouse_x, mouse_y), ssTL));
            if (dist < shortestMouseVertexDistance) {
                shortestMouseVertexDistance = dist;
                currentSelectedPrimObject = po;
                po->hasCurrentSelectedVertex = true;
                po->selectedVertexScreenCoordinates = ssTL;
            }

            dist = glm::distance(glm::vec2(mouse_x, mouse_y), ssBL);
            if (dist < shortestMouseVertexDistance) {
                shortestMouseVertexDistance = dist;
                currentSelectedPrimObject = po;
                po->hasCurrentSelectedVertex = true;
                po->selectedVertexScreenCoordinates = ssBL;
            }

            dist = glm::distance(glm::vec2(mouse_x, mouse_y), ssBR);
            if (dist < shortestMouseVertexDistance) {
                shortestMouseVertexDistance = dist;
                currentSelectedPrimObject = po;
                po->hasCurrentSelectedVertex = true;
                po->selectedVertexScreenCoordinates = ssBR;
            }

            dist = glm::distance(glm::vec2(mouse_x, mouse_y), ssTR);
            if (dist < shortestMouseVertexDistance) {
                shortestMouseVertexDistance = dist;
                currentSelectedPrimObject = po;
                po->hasCurrentSelectedVertex = true;
                po->selectedVertexScreenCoordinates = ssTR;
            }

        }
    }

    // Render the currently selected
    if (currentSelectedPrimObject) {
        location({currentSelectedPrimObject->selectedVertexScreenCoordinates.x, currentSelectedPrimObject->selectedVertexScreenCoordinates.y, -0.6});
        scale({2, 2, 1});
        foregroundColor({0.9, 0.9, 0, 1});
        drawPlane();
    }




}

void DungeonGame::renderPhysicsObjects() {
    for (auto po : _physicsObjects) {
        bindCamera(getGameplayCamera());
        lightingOn();
        location(po->getPosition());
        scale({1, 1, 1});
        foregroundColor({1, 0, 0, 1});
        rotation({0, 0, 0});
        bindMesh(po->getMesh());
        drawMesh();
        rotation({0, 0, 0});
    }
}

void DungeonGame::render() {
    bindCamera(getGameplayCamera());
   lightingOn();
//
//    scale({1, 1,1});
//    foregroundColor({0.3, 0.6, 0.2, .1});
//    location(glm::vec3{0, 0, 0});
//    gridLines(100);
//    drawGrid();
//
//    location({10, 0, -2});
//    bindMesh(getMeshByName("hero"));
//    //bindTexture(getTextureByName("xbox"));
//    bindTexture(nullptr);
//    foregroundColor({0.1, 0.1, 0.9, 1});
//    drawMesh();
//
//    renderPrimObjects();
//
//    renderPhysicsObjects();

    // This is an example how we can embed an editor on top of our game.
    if (showEditor) {
        editor->renderImGui();
    }

    //renderFPS();

}

void DungeonGame::update() {

    // First check the basic function keys which can turn on the editor etc.
    if (keyPressed(VK_F11)) {
        showEditor = !showEditor;
    }

    // We move the camera with the WASD keys
    //and potentially later with a gamepad right stick.
    {

        float camspeed = 0.007;
        float dir = 0;
        float hdir = 0;
        float yaw = 0;

        if (isKeyDown('E')) {
            yaw = -1;
        }

        if (isKeyDown('Q')) {
            yaw = 1;
        }

        if (isKeyDown('W')) {
            dir = 1;
        }
        if (isKeyDown('S')) {
            dir = -1;
        }

        if (isKeyDown('A')) {
            hdir = -1;
        }
        if (isKeyDown('D')) {
            hdir = 1;
        }

        auto cam = getGameplayCamera();
        auto fwd = camFwd(cam);
        auto right = cameraRightVector(cam);

        // Update fwd with yaw
        glm::vec4 fwdAfterYaw = glm::rotate(glm::mat4(1), yaw * camspeed * 0.1f, glm::vec3(0, 1, 0)) * glm::vec4(fwd, 0);

        glm::vec3 loc = cam->location;
        loc += glm::vec3{camspeed * fwdAfterYaw.x, 0, camspeed * fwdAfterYaw.z} * dir;
        loc += glm::vec3{camspeed * right.x * 0.33, 0, camspeed * right.z * 0.33} * hdir;
        auto tgt = loc + glm::vec3{fwdAfterYaw.x, fwdAfterYaw.y, fwdAfterYaw.z};
        getGameplayCamera()->updateLocation({loc.x, loc.y, loc.z});
        getGameplayCamera()->updateLookupTarget({tgt.x, tgt.y, tgt.z});
    }

    // Physics update
    {
        auto ftSecs = (float)ftMicros / 1000.0 / 1000.0;
        for (auto po : _physicsObjects) {
            po->update(ftSecs);
            if (po->getPosition().y < 0) {
                // Of course this is just a hack to "emulate" things staying grounded.
                po->stopSimulation();
            }
        }
    }


}

bool DungeonGame::shouldStillRun() {
    return true;
}

std::string DungeonGame::getAssetFolder() {
    return "../games/dungeon_shoot/assets";
}

bool DungeonGame::shouldAutoImportAssets() {
    return true;
}


DefaultGame* getGame() {
    return new DungeonGame();
}




