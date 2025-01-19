//
// Created by mgrus on 15.09.2024.
//

#include "editor.h"
#include "graphics.h"

#include "../game/game_model.h"

namespace editor {
    void Editor::renderImGui() {

        static bool open_new_level_popup = false;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                // Now you can add MenuItems or other ImGui calls here
                if (ImGui::MenuItem("New Level")) {
                    open_new_level_popup = true;
                    // TODO ask if the current level should be saved?
                    level = new Level();

                }

                if (ImGui::MenuItem("Open Level")) {
                    // TODO ask if the current level should be saved?
                    // TODO show file dialog for levels

                }
                if (ImGui::MenuItem("Save Level")) {
                    saveLevel();
                }
                ImGui::EndMenu();  // Close the 'File' menu
            }
            renderMainMenu();
            ImGui::EndMainMenuBar();  // Close the main menu bar
        }

        drawTopToolbar();
        ShowGameEditor();

        renderMouseCoordinates();

        if (open_new_level_popup) {
            // Open the popup when the flag is set
            ImGui::OpenPopup("New Level Popup");
            open_new_level_popup = false;  // Reset the flag
        }

        if (ImGui::BeginPopupModal("New Level Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

            ImGui::Text("Name of level:");
            ImGui::SameLine();
            char buf[150];
            ZeroMemory(buf, 150);
            ImGui::InputText("##levelName", buf, 150);

            if (ImGui::Button("GO!")) {
                state->editingLevel = true;
                ImGui::SetWindowPos("GameEditor", ImVec2(0, 100));
                ImGui::SetWindowSize("GameEditor", ImVec2(300, window_height - 30));
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

    }

    void Editor::update() {
        if (keyPressed(VK_F3)) {
            doImportMeshAction();
        }
    }

    void Editor::saveLevel() {
        // TODO implement
    }

    void Editor::renderMouseCoordinates() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always); // Set size to 0 (will resize automatically)
        ImGui::SetNextWindowBgAlpha(0.0f); // Make window transparent
        ImGui::SetNextWindowPos({window_width - 130.0f, window_height - 30.0f});
        ImGui::Begin("Invisible Window", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration);
        ImGui::Text("mouse: %d/%d", mouse_x, mouse_y);
        ImGui::End();
        ImGui::PopStyleVar();
    }

    Camera *Editor::getMeshViewerCamera() {
        if (!_meshViewerCamera) {
            _meshViewerCamera = new Camera();
            _meshViewerCamera->location = {0, 2, 3};
            _meshViewerCamera->lookAtTarget = {0, 0, -5};
            _meshViewerCamera->type = CameraType::Perspective;
        }

        return _meshViewerCamera;
    }

    void Editor::renderMeshViewerExt() {
        if (!importedMesh) {
            return;
        }

        cameraMover->update();
        ImGui::Begin("Mesh Viewer");
        //ImGui::SetWindowSize("Mesh Viewer", {1000, 600});
        activateFrameBuffer(skeletalMeshWindowFrameBuffer);
        bindCamera(getMeshViewerCamera());
        glViewport(0, 0, skeletalMeshWindowFrameBuffer->texture->bitmap->width,
                 skeletalMeshWindowFrameBuffer->texture->bitmap->height);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scale({1, 1, 1});
        location({0, 0, 0});
        foregroundColor({0.3, 0.6, 0.2, .1});
        gridLines(100);
        drawGrid();

        // Draw the imported mesh,
        // regardless if this is a static or skeleton mesh
        {

            bindMesh(importedMesh);
            location({0, 0, 0});
            rotation({0, 0, 0});
            scale({1, 1, 1});
            foregroundColor({0.9, 0.9, 0.9, 1});

            // These 2 lines are good (necessary?) for avoiding z-fighting with the following wireframe mesh
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0f, 1.0f);

            if (currentAnimation && animationPlaying) {
                setSkinnedDraw(true);
                animationPlayer->update();
            }

            // Draw the filled mesh
            drawMesh();

            // Now draw the wireframe version
            wireframeOn();
            lightingOff();
            foregroundColor({0.9, 0.1, 0.1, 1});
            drawMesh();
            wireframeOff();
            lightingOn();

            if (currentAnimation && animationPlaying) {
                setSkinnedDraw(false);
            }
        }

        // Draw the skeleton if the mesh has one
        {
            if (importedMesh->skeleton) {
                lightingOff();
                bindMesh(assetLoader->getMesh("bone_mesh"));
                foregroundColor({0.5, 0.5, 1, 1});

                int jointCounter = 0;
                if (importedMesh->skeleton) {
                    // To actually see the bones in front of the mesh, we turn depth test off:
                    depthTestOff();

                    for (auto j: importedMesh->skeleton->joints) {
                        auto finalTransform = j->globalTransform;
                        if (currentAnimation) {
                            finalTransform = animationPlayer->calculateFramePoseForJoint(currentAnimationFrame, j);
                        }

                        useWorldMatrix(true);
                        setWorldMatrix(finalTransform);
                        foregroundColor({0.7, 0.7, 0, 1});
                        jointCounter++;
                        //scale(glm::vec3(1));
                        drawMesh();
                    }

                    // Reset our render state back to normal:
                    depthTestOn();
                    useWorldMatrix(false);
                    lightingOn();

                }
            }
        }

        // Rest to normal viewport:
        glViewport(0, 0, window_width, window_height);

        // Activate main framebuffer again:
        activateFrameBuffer(nullptr);

        // We render a 2-column table, with the 3D window on the left, and another table with
        // detailed skeleton information on the right.
        if (ImGui::BeginTable("TwoColumnTable", 2)) {

            ImGui::TableNextColumn();
            // Now we are finished rendering our 3D scene into our framebuffer.
            // Next we present the texture of the FBO as an image in ImGUI.
            ImGui::Image(reinterpret_cast<ImTextureID>(skeletalMeshWindowFrameBuffer->texture->handle),
                             {(float) skeletalMeshWindowFrameBuffer->texture->bitmap->width / 2,
                              (float) skeletalMeshWindowFrameBuffer->texture->bitmap->height / 2},
                             {0, 1}, {1, 0});

            ImGui::TableNextColumn();
            ImGui::Text("Joint infos");

            if (ImGui::BeginTable("bone_detail_table", 4)) {
                // Set up headers for the table (optional)
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("localTransform", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("offsetMatrix", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Parent");
                ImGui::TableHeadersRow();

                for (auto j: importedMesh->skeleton->joints) {

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); // First column
                    ImGui::Text("%s", j->name.c_str());

                    ImGui::TableSetColumnIndex(1); // Second column
                    float x = j->localTransform[3][0]; // Translation x
                    float y = j->localTransform[3][1]; // Translation y
                    float z = j->localTransform[3][2]; // Translation z
                    ImGui::Text("%f/%f/%f", x, y, z);

                    ImGui::TableSetColumnIndex(2); // Second column
                    float ibx = j->inverseBindMatrix[3][0]; // Translation x
                    float iby = j->inverseBindMatrix[3][1]; // Translation y
                    float ibz = j->inverseBindMatrix[3][2]; // Translation z
                    ImGui::Text("%f/%f/%f", ibx, iby, ibz);

                    ImGui::TableSetColumnIndex(3);
                    if (j->parent) {
                        ImGui::Text(j->parent->name.c_str());
                    } else {
                        ImGui::Text("-");
                    }

                }

                // End the table
                ImGui::EndTable();
            }

            // Render current animation details:
            if (currentAnimation) {
                ImGui::Text("# Frames: %d", currentAnimation->frames);
                if (ImGui::BeginTable("anim_detail_table", 5)) {

                    ImGui::TableSetupColumn("Time");
                    ImGui::TableSetupColumn("Joint");
                    ImGui::TableSetupColumn("Translation", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Rotation", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();


                    for (auto pose: currentAnimation->samplesPerJoint) {

                        for (auto sample: *pose.second) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("%f ", sample->time);

                            ImGui::TableNextColumn();
                            ImGui::Text("%s", sample->jointName.c_str());

                            ImGui::TableNextColumn();
                            ImGui::Text("%f/%f/%f", sample->translation.x, sample->translation.y, sample->translation.z);

                            ImGui::TableNextColumn();
                            ImGui::Text("%f/%f/%f", sample->rotation.x, sample->rotation.y, sample->rotation.z);

                        }


                    }

                    ImGui::EndTable();
                }
            }


            ImGui::EndTable();
        }


        ImGui::End();
    }

    void Editor::renderMeshViewer() {
        if (!importedMesh) {
            return;
        }

        cameraMover->update();

        ImGui::Begin("Mesh Viewer");
        ImGui::SetWindowSize("Mesh Viewer", {1000, 600});
        // We render our imported skeletal mesh into a texture, which we then show as an image:
        activateFrameBuffer(skeletalMeshWindowFrameBuffer);
        bindCamera(getMeshViewerCamera());

        glViewport(0, 0, skeletalMeshWindowFrameBuffer->texture->bitmap->width,
                   skeletalMeshWindowFrameBuffer->texture->bitmap->height);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scale({1, 1, 1});
        location({0, 0, 0});
        foregroundColor({0.3, 0.6, 0.2, .1});
        gridLines(100);
        drawGrid();


        static bool showMesh = true;
        static bool showAsWireFrame = false;
        static bool showSkeleton = false;
        static float meshScale = .01;

        if (showMesh) {

            // Draw in red if no texture is set.
            foregroundColor({1, 0, 0, 1});

            // We bind a texture if it is not null, null otherwise
            bindTexture(importedTexture);

            // Make sure to display the texture correctly
            flipUvs(true);

            bindMesh(importedMesh);
            location({0, 0, 0});
            rotation({0, 0, 0});

            if (importedMesh->skeleton) {
                setSkinnedDraw(true);
                if (showAsWireFrame) {
                    lightingOff();
                    wireframeOn();
                }

#ifdef SEFPARATE_ANIM_IMPL
                std::vector<glm::mat4> boneMatrices;
                for (auto j: importedMesh->skeleton->joints) {
                    if (currentAnimation) {
                        if (currentAnimation->samplesPerJoint[j->name] != nullptr &&
                            (currentAnimation->samplesPerJoint[j->name]->size() > currentAnimationFrame)) {
                            auto sample = (*currentAnimation->samplesPerJoint[j->name])[currentAnimationFrame];
                            if (sample) {
                                j->localTransform = glm::translate(glm::mat4(1), sample->translation) *
                                                    glm::toMat4(sample->rotation);
                                j->globalTransform = calculateWorldTransform(j, j->localTransform);
                                j->finalTransform = j->globalTransform * j->inverseBindMatrix;
                            }
                        }

                    }
                    boneMatrices.push_back(j->finalTransform);

                }
                setBoneMatrices(boneMatrices);
#endif

                animationPlayer->update();

            }

            scale({meshScale, meshScale, meshScale});
            drawMesh();

            setSkinnedDraw(false);
            if (showAsWireFrame) {
                wireframeOff();
                lightingOn();
            }
        }

        // Track the selected joint in our table:
        static int selectedJoint = -1;
        if (showSkeleton) {
            // Draw the skeleton
            bindMesh(assetLoader->getMesh("bone_mesh"));
            foregroundColor({0.5, 0.5, 1, 1});

            int jointCounter = 0;
            if (importedMesh->skeleton) {
                // For rendering the joints we tweak the render state a bit:
                if (showMesh) {
                    depthTestOff();
                }

                for (auto j: importedMesh->skeleton->joints) {
                    // We use the worldMatrix as is instead
                    // of pulling out manually location, scale, rotation from the matrix.
                    useWorldMatrix(true);
                    setWorldMatrix(j->globalTransform);

                    // Color the selected joint differently
                    if (jointCounter == selectedJoint) {
                        foregroundColor({1, 1, 0, 1});
                    } else {
                        foregroundColor({0.5, 0.5, 1, 1});
                    }
                    jointCounter++;

                    scale(glm::vec3(1));
                    drawMesh();
                }

                // Reset our render state back to normal:
                depthTestOn();
                useWorldMatrix(false);

            }
        }


        // Rest to normal viewport:
        glViewport(0, 0, window_width, window_height);

        // Activate main framebuffer again:
        activateFrameBuffer(nullptr);

        // We have a 2 column layout in our editor:
        // Left is the visualisation of the mesh,
        // right is some tabular information (properties):
        if (ImGui::BeginTable("TwoColumnTable", 2)) {

            // Left column
            ImGui::TableNextColumn();
            ImGui::Image(reinterpret_cast<ImTextureID>(skeletalMeshWindowFrameBuffer->texture->handle),
                         {(float) skeletalMeshWindowFrameBuffer->texture->bitmap->width / 2,
                          (float) skeletalMeshWindowFrameBuffer->texture->bitmap->height / 2},
                         {0, 1}, {1, 0});


            // Right column
            ImGui::TableNextColumn();
            ImGui::Text("Model: %s", importedMesh->fileName.c_str());

            ImGui::Checkbox("Skeleton", &showSkeleton);
            ImGui::Checkbox("Mesh", &showMesh);
            ImGui::SameLine();
            static char scaleBuf[5];
            sprintf(scaleBuf, std::to_string(meshScale).c_str());
            ImGui::InputText("##scale", scaleBuf, 5);
            meshScale = atof(scaleBuf);
            ImGui::Checkbox("Wireframe", &showAsWireFrame);

            int row = 0;
            if (importedMesh->skeleton) {
                ImGui::Text("Bones: %d", importedMesh->skeleton->joints.size());


                if (ImGui::BeginTable("bone_detail_table", 3)) {
                    // Set up headers for the table (optional)
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("offsetMatrix");
                    ImGui::TableSetupColumn("Parent");
                    ImGui::TableHeadersRow();

                    for (auto j: importedMesh->skeleton->joints) {

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); // First column

                        bool isSelected = (selectedJoint == row);
                        if (ImGui::Selectable(j->name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                            selectedJoint = row;
                        }
                        row++;

                        ImGui::TableSetColumnIndex(1); // Second column
                        float x = j->globalTransform[3][0]; // Translation x
                        float y = j->globalTransform[3][1]; // Translation y
                        float z = j->globalTransform[3][2]; // Translation z
                        ImGui::Text("%f/%f/%f", x, y, z);

                        ImGui::TableSetColumnIndex(2);
                        if (j->parent) {
                            ImGui::Text(j->parent->name.c_str());
                        } else {
                            ImGui::Text("-");
                        }

                    }

                    // End the table
                    ImGui::EndTable();
                }
            }

            ImGui::EndTable();

        }

        ImGui::End();
    }

    void Editor::renderMainMenu() {
        renderGameObjectsMenu();
        renderMeshMenu();
    }

    std::string Editor::showFileDialog(const std::string &typeFilter) {

        OPENFILENAME ofn;       // Common dialog box structure
        char szFile[260] = {0}; // Buffer for file name
        HWND hwnd = NULL;       // Owner window
        HANDLE hf;              // File handle

        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = typeFilter.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        // Display the Open dialog box
        if (GetOpenFileName(&ofn) == TRUE) {
            std::cout << "Selected file: " << ofn.lpstrFile << std::endl;
            return ofn.lpstrFile;

        } else {
            std::cout << "No file selected or operation canceled." << std::endl;
            return "";
        }
    }

    void Editor::renderMeshMenu() {
        if (ImGui::BeginMenu("Mesh")) {
            if (ImGui::MenuItem("Import Mesh")) {
                doImportMeshAction();
            }

            if (lastImportedMeshFileName != "") {
                if (ImGui::MenuItem("Reimport last mesh")) {
                    if (importedMesh) {
                        delete (importedMesh);
                    }
                    importedMesh = MeshImporter().importMesh(lastImportedMeshFileName);
                }
            }

            if (ImGui::MenuItem("Load Texture")) {
                lastImporteTextureFileName = showFileDialog("All\0*.*\0png\0*.png\0jpg\0*.jpg\0bmp\0*.bmp");
                if (lastImporteTextureFileName != "") {
                    if (importedTexture) {
                        delete (importedTexture);
                    }
                    importedTexture = createTextureFromFile(lastImporteTextureFileName);
                }
            }

            if (ImGui::MenuItem("Import Animation")) {
                auto fileName = showFileDialog("All\0*.*\0fbx\0*.fbx\0gltf\0*.glb");
                if (fileName != "") {
                    auto importedAnims = MeshImporter().importAnimations(fileName);
                    for (auto anim: importedAnims) {
                        importedAnimations[anim->name] = anim;
                    }
                }
            }
            ImGui::EndMenu();
        }
    }

    void Editor::renderGameObjectsMenu() {
        static int numCubes = 0;
        static int numSpheres = 0;
        static int numCharacters = 0;
        if (ImGui::BeginMenu("Game Objects")) {
            if (ImGui::BeginMenu("New ...")) {
                // Now you can add MenuItems or other ImGui calls here
                if (ImGui::MenuItem("Cube")) {
                    auto go = new GameObject();
                    go->name = "cube_" + std::to_string(numCubes++);
                    if (currentSelectedGameObjectInTree) {
                        currentSelectedGameObjectInTree->children.push_back(go);
                    } else {
                        level->gameObjects.push_back(go);
                    }

                }
                if (ImGui::MenuItem("Sphere")) {

                }
                if (ImGui::MenuItem("Character")) {

                }
                ImGui::EndMenu();  // Close the 'New...' menu
            }
            ImGui::EndMenu();  // Close the 'Game Objects' menu
        }
    }

    void Editor::doImportMeshAction() {
        lastImportedMeshFileName = showFileDialog("All\0*.*\0obj\0*.obj\0fbx\0*.fbx\0gltf\0*.gltf");
        if (lastImportedMeshFileName != "") {
            if (importedMesh) {
                delete (importedMesh);
            }
            importedMesh = MeshImporter().importMesh(lastImportedMeshFileName);
            animationPlayer->setMesh(importedMesh);
        }
    }

    void Editor::recurseRenderGameObjects(GameObject *parent) {
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (currentSelectedGameObjectInTree == parent) {
            nodeFlags |= ImGuiTreeNodeFlags_Selected;  // Mark as selected
        }

        bool parentOpen = ImGui::TreeNodeEx(parent->name.c_str(), nodeFlags);

        if (ImGui::IsItemClicked()) {
            currentSelectedGameObjectInTree = parent;  // Select the current node
        }

        // If the node is open, process its children
        if (parentOpen) {
            for (auto c: parent->children) {
                recurseRenderGameObjects(c);
            }
            ImGui::TreePop();  // End the tree node
        }

    }

// All windows related
    void Editor::ShowGameEditor() {

        // Game object as tree
        ImGui::Begin("Game Objects");
        for (auto go: level->gameObjects) {
            recurseRenderGameObjects(go);
        }
        ImGui::End();

        ImGui::Begin("Properties");
        if (currentSelectedGameObjectInTree) {
            ImGui::Text("Current object: %s", currentSelectedGameObjectInTree->name.c_str());
        } else {
            ImGui::Text("Current object: %s", "None");
        }

        ImGui::End();

        ImGui::Begin("Animations");
        drawAnimationTimeline();
        ImGui::End();

        renderMeshViewerExt();


    }

    void Editor::drawTopToolbar() {
        ImGui::Begin("Toolbar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowPos(ImVec2(0, 30));     // Position at the top
        float toolbarHeight = 40;
        if (ImGui::ImageButton((void *) assetLoader->getTexture("button_play")->handle, {16, 16})) {
            printf("play button pressed\n");
        }
        ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, toolbarHeight));  // Full window width, height of 50
        ImGui::End();
    }

    void Editor::drawAnimationTimeline() {

        ImGui::BeginChild("Toolbar", ImVec2(ImGui::GetIO().DisplaySize.x, 40),
                          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        float toolbarHeight = 40;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
        float GRID_SZ = 64.0f;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        int beginTimeLineX = win_pos.x + 100;

        // Vertical lines main
        for (float x = fmodf(0, GRID_SZ); x < canvas_sz.x; x += GRID_SZ) {
            auto s = ImVec2(x + beginTimeLineX, 0.0f + win_pos.y);
            auto e = ImVec2(x + beginTimeLineX, canvas_sz.y + win_pos.y);
            draw_list->AddLine(s, e, GRID_COLOR);
        }

        // Vertical lines small
        for (float x = fmodf(0, GRID_SZ); x < canvas_sz.x; x += GRID_SZ) {
            auto s = ImVec2(32 + x + beginTimeLineX, (toolbarHeight - 20.0f) + win_pos.y);
            auto e = ImVec2(32 + x + beginTimeLineX, canvas_sz.y + win_pos.y);
            draw_list->AddLine(s, e, GRID_COLOR);
        }


        // Horizontal lines
//        for (float y = fmodf(0, GRID_SZ); y < canvas_sz.y; y += GRID_SZ) {
//            auto s = ImVec2(0 + win_pos.x, y+ win_pos.y);
//            auto e = ImVec2( canvas_sz.x+ win_pos.x, y + win_pos.y);
//            draw_list->AddLine(s,e, GRID_COLOR);
//        }

        // Draw line numbers as text
        for (int x = 0; x < 20; x++) {
            draw_list->AddText({3 + 64.0f * (float) x + beginTimeLineX, win_pos.y + (toolbarHeight - 22)},
                               IM_COL32(100, 120, 10, 150), std::to_string(x).c_str());
        }

        ImGui::EndChild();
        // Close the toolbar window


        ImGui::Text("Animations");
        ImGui::Text("Current Active animation: %s",
                    (currentAnimation != nullptr ? currentAnimation->name.c_str() : "--"));
        ImGui::Checkbox("Loop", &animationLooped);
        if (ImGui::Button("Play")) {
            animationPlaying = true;
            animationPlayer->play(animationLooped);
        }
        if (ImGui::Button("Stop")) {
            animationPlaying = false;
        }
        if (importedMesh) {
            auto animations = importedMesh->animations;
            int sampleOffsetY = 0;
            for (auto anim: animations) {

                std::vector<std::string> joints;
                // Iterate over the map and collect keys
                for (const auto &pair: anim->samplesPerJoint) {
                    joints.push_back(pair.first);
                }


                sampleOffsetY += 16;
                ImGui::Text("Name: %s Duration: %f", anim->name.c_str(), anim->duration);
                ImGui::SameLine();
                if (ImGui::Button("<")) {
                    if (currentAnimationFrame > 0) {
                        currentAnimationFrame--;
                    }

                }
                ImGui::SameLine();
                if (ImGui::Button(">")) {
                    currentAnimationFrame++;
                }
                ImGui::SameLine();
                ImGui::Text("Current frame: %d", currentAnimationFrame);
                ImGui::SameLine();
                //ImGui::Text("time: %f", (*anim->samplesPerJoint[joints[0]])[currentAnimationFrame]->time);
                ImGui::SameLine();
                std::string label = "Activate##" + anim->name;
                if (ImGui::Button(label.c_str())) {
                    currentAnimation = anim;
                    animationPlayer->switchAnimation(currentAnimation);
                    currentAnimationFrame = 0;

                }
                ImGui::SameLine();
                if (ImGui::Button("Deactivate")) {
                    animationPlayer->switchAnimation(nullptr);
                    animationPlayer->stop();
                    currentAnimation = nullptr;
                    currentAnimationFrame = 0;
                }
                // Draw samples into timeline
//            for (auto s : anim->samples) {
//                draw_list->AddCircle({ (s->time/2) + beginTimeLineX, sampleOffsetY + win_pos.y + (toolbarHeight)}, 8, GRID_COLOR, 4);
//            }
            }
        }

        static float animTime = 0;
        static float frameTime = 0;
        if (currentAnimation && animationPlaying) {
            animTime += (ftSeconds * 1000.0f);
            frameTime += (ftSeconds * 1000.0f);

            // TODO: Interpolate the key frames according to the current time
            // in between the two frames we are.
            // Currently we just update at 30fps
            if (frameTime > 33.3333) {
                currentAnimationFrame++;
                frameTime = 0;
            }

            if (currentAnimationFrame > (currentAnimation->frames - 1)) {
                currentAnimationFrame = 0;
                animTime = 0;
                if (!animationLooped) {
                    animationPlaying = false;
                }
            }
        }


    }

    Editor::Editor() {
        state = new EditorState();
        level = new Level();

        // Create images (textures):
        //playButtonTexture = createTextureFromFile("../src/engine/editor/assets/button_play.png");
        assetLoader = new FolderAssetLoader();
        assetLoader->load("../src/engine/editor/assets");
        skeletalMeshWindowFrameBuffer = createFrameBuffer(1024, 1024);
        cameraMover = new CameraMover(getMeshViewerCamera());
        cameraMover->setMovementSpeed(30);
        animationPlayer = new AnimationPlayer(nullptr, nullptr);
    }

    void EditorGame::init() {
        DefaultGame::init();

        editor = new Editor();
    }

    void EditorGame::update() {

        editor->update();
    }

    void EditorGame::render() {
        editor->renderImGui();
    }

    bool EditorGame::shouldStillRun() {
        return true;
    }
}

//DefaultGame* getGame() {
//    return nullptr;
//}