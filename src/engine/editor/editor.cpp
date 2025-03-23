//
// Created by mgrus on 15.09.2024.
//

#include "editor.h"

#include <codecvt>
#include <ranges>
#include <engine/animation/AnimationBlender.h>
#include <engine/animation/BoneMatrixCalculator.h>
#include <engine/animation/JointMask.h>
#include <engine/animation/PerBoneBlendData.h>
#include <engine/animation/Pose.h>
#include <engine/rastergraphics/rastergraphics.h>
#include <shobjidl.h>

#include "graphics.h"

#include "../game/game_model.h"
#include "ozz/include/ozz/animation/runtime/local_to_model_job.h"
#include "ozz/include/ozz/animation/runtime/skeleton.h"
#include "ozz/include/ozz/base/span.h"
#include "ozz/include/ozz/base/maths/simd_math.h"
#include "ozz/include/ozz/base/maths/soa_float.h"
#include "ozz/include/ozz/base/maths/soa_transform.h"
#include "ozz/include/ozz/base/maths/transform.h"
#include "ozz/include/ozz/base/maths/internal/simd_math_config.h"

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
            _meshViewerCamera->location = {-2, 3, 6};
            _meshViewerCamera->lookAtTarget = {1, 1, 0};
            _meshViewerCamera->type = CameraType::Perspective;
        }

        return _meshViewerCamera;
    }

    /**
    * This method takes a SoA transform as input and unpacks it into an AoS transform.
    * As each SoA struct holds 4 AoS structs, we also need to give the index (0 - 3) which we actually want to extract here.
    */
    ozz::math::Transform Editor::convertSoaToAos(const ozz::math::SoaTransform &soa_transform, int element) {
        ozz::math::Transform joint_transform;

        
        ozz::math::SimdFloat4 translations_x = soa_transform.translation.x;
        ozz::math::SimdFloat4 translations_y = soa_transform.translation.y;
        ozz::math::SimdFloat4 translations_z = soa_transform.translation.z;

        // Similarly for rotation
        ozz::math::SimdFloat4 rotations_x = soa_transform.rotation.x;
        ozz::math::SimdFloat4 rotations_y = soa_transform.rotation.y;
        ozz::math::SimdFloat4 rotations_z = soa_transform.rotation.z;
        ozz::math::SimdFloat4 rotations_w = soa_transform.rotation.w;

        // Similarly for scale
        ozz::math::SimdFloat4 scales_x = soa_transform.scale.x;
        ozz::math::SimdFloat4 scales_y = soa_transform.scale.y;
        ozz::math::SimdFloat4 scales_z = soa_transform.scale.z;

        // Extract single float from SimdFloat4.
        // To actually get to a usable float value
        if (element == 0) {
            joint_transform.translation.x = ozz::math::GetX(ozz::math::SplatX(translations_x));
            joint_transform.translation.y = ozz::math::GetX(ozz::math::SplatX(translations_y));
            joint_transform.translation.z = ozz::math::GetX(ozz::math::SplatX(translations_z));

            joint_transform.rotation.x = ozz::math::GetX(ozz::math::SplatX(rotations_x));
            joint_transform.rotation.y = ozz::math::GetX(ozz::math::SplatX(rotations_y));
            joint_transform.rotation.z = ozz::math::GetX(ozz::math::SplatX(rotations_z));
            joint_transform.rotation.w = ozz::math::GetX(ozz::math::SplatX(rotations_w));

            joint_transform.scale.x = ozz::math::GetX(ozz::math::SplatX(scales_x));
            joint_transform.scale.y = ozz::math::GetX(ozz::math::SplatX(scales_y));
            joint_transform.scale.z = ozz::math::GetX(ozz::math::SplatX(scales_z));
        }
        else if (element == 1) {
            joint_transform.translation.x = ozz::math::GetX(ozz::math::SplatY(translations_x));
            joint_transform.translation.y = ozz::math::GetX(ozz::math::SplatY(translations_y));
            joint_transform.translation.z = ozz::math::GetX(ozz::math::SplatY(translations_z));

            joint_transform.rotation.x = ozz::math::GetX(ozz::math::SplatY(rotations_x));
            joint_transform.rotation.y = ozz::math::GetX(ozz::math::SplatY(rotations_y));
            joint_transform.rotation.z = ozz::math::GetX(ozz::math::SplatY(rotations_z));
            joint_transform.rotation.w = ozz::math::GetX(ozz::math::SplatY(rotations_w));

            joint_transform.scale.x = ozz::math::GetX(ozz::math::SplatY(scales_x));
            joint_transform.scale.y = ozz::math::GetX(ozz::math::SplatY(scales_y));
            joint_transform.scale.z = ozz::math::GetX(ozz::math::SplatY(scales_z));
        }
        else if (element == 2) {
            joint_transform.translation.x = ozz::math::GetX(ozz::math::SplatZ(translations_x));
            joint_transform.translation.y = ozz::math::GetX(ozz::math::SplatZ(translations_y));
            joint_transform.translation.z = ozz::math::GetX(ozz::math::SplatZ(translations_z));

            joint_transform.rotation.x = ozz::math::GetX(ozz::math::SplatZ(rotations_x));
            joint_transform.rotation.y = ozz::math::GetX(ozz::math::SplatZ(rotations_y));
            joint_transform.rotation.z = ozz::math::GetX(ozz::math::SplatZ(rotations_z));
            joint_transform.rotation.w = ozz::math::GetX(ozz::math::SplatZ(rotations_w));

            joint_transform.scale.x = ozz::math::GetX(ozz::math::SplatZ(scales_x));
            joint_transform.scale.y = ozz::math::GetX(ozz::math::SplatZ(scales_y));
            joint_transform.scale.z = ozz::math::GetX(ozz::math::SplatZ(scales_z));
        }
        else if (element == 3) {
            joint_transform.translation.x = ozz::math::GetX(ozz::math::SplatW(translations_x));
            joint_transform.translation.y = ozz::math::GetX(ozz::math::SplatW(translations_y));
            joint_transform.translation.z = ozz::math::GetX(ozz::math::SplatW(translations_z));

            joint_transform.rotation.x = ozz::math::GetX(ozz::math::SplatW(rotations_x));
            joint_transform.rotation.y = ozz::math::GetX(ozz::math::SplatW(rotations_y));
            joint_transform.rotation.z = ozz::math::GetX(ozz::math::SplatW(rotations_z));
            joint_transform.rotation.w = ozz::math::GetX(ozz::math::SplatW(rotations_w));

            joint_transform.scale.x = ozz::math::GetX(ozz::math::SplatW(scales_x));
            joint_transform.scale.y = ozz::math::GetX(ozz::math::SplatW(scales_y));
            joint_transform.scale.z = ozz::math::GetX(ozz::math::SplatW(scales_z));
        }


        return joint_transform;

    }

    void Editor::renderMeshViewerExt() {
        if (!importedMesh) {
            return;
        }

        cameraMover->update();
        ImGui::Begin("Mesh Viewer");
        //ImGui::SetWindowSize("Mesh Viewer", {1000, 600});
        activateFrameBuffer(skeletalMeshWindowFrameBuffer.get());
        bindCamera(getMeshViewerCamera());
        glViewport(0, 0, skeletalMeshWindowFrameBuffer->texture->bitmap->width,
                 skeletalMeshWindowFrameBuffer->texture->bitmap->height);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static auto gridData = createGrid(100);
        gridData->camera = getMeshViewerCamera();
        gridData->color = glm::vec4(.5, .5, 0, 1);
        gridData->scale = 0.25f;
        drawGrid(gridData, glm::ivec2{skeletalMeshWindowFrameBuffer->texture->bitmap->width, skeletalMeshWindowFrameBuffer->texture->bitmap->height});


        wireframeOn(3.0f);
        gridData->scale = 1.0f;
        drawGrid(gridData, glm::ivec2{skeletalMeshWindowFrameBuffer->texture->bitmap->width, skeletalMeshWindowFrameBuffer->texture->bitmap->height});
        wireframeOff();



        // Draw the imported mesh,
        // regardless if this is a static or skeleton mesh
        {

            // Basic directional light for the render
            Light* sun = new Light();
            sun->type = LightType::Directional;
            sun->location = {-1, 5, 3};
            sun->lookAtTarget = {0, 0, 0};
            sun->calculateDirectionFromCurrentLocationLookat();
            sun->castsShadow = false;

            // Draw the filled mesh
            MeshDrawData dd;
            dd.mesh = importedMesh;
            dd.viewPortDimensions = glm::ivec2{skeletalMeshWindowFrameBuffer->texture->bitmap->width, skeletalMeshWindowFrameBuffer->texture->bitmap->height};
            dd.location = {0, 0, 0};
            dd.camera = getMeshViewerCamera();
            dd.color = {0.9, 0.9, 0.9, 1};
            dd.directionalLights.push_back(sun);

            if (blendedAnimations.size() > 1 && !animationPlaying) {
                // Find the pose for each blended animation
                // And then - blend it!!!
                Pose* finalPose = new Pose();


                // Create an effective override of the upper body for our aim/walk blend.
                // Only the aim should contribute to the upper body bones.
                PerBoneBlendData perBoneBlendData;
                perBoneBlendData.addNamedBoneWeight("upperarm.r", 1);
                perBoneBlendData.addNamedBoneWeight("lowerarm.r", 1);
                perBoneBlendData.addNamedBoneWeight("hand.r", 1);
                perBoneBlendData.addNamedBoneWeight("upperarm.l", 1);
                perBoneBlendData.addNamedBoneWeight("lowerarm.l", 1);
                perBoneBlendData.addNamedBoneWeight("hand.l", 1);

                // Always blend the first 2 animations for now:
                auto blendedAnimsList = std::vector(blendedAnimations.begin(), blendedAnimations.end());
                finalPose = BoneMatrixCalculator().calculateBlendedPose(blendedAnimsList[0], blendedAnimsList[1], importedMesh->skeleton, blendTimestamp, blendWeight, &perBoneBlendData);

                if (finalPose) {
                    dd.skinnedDraw = true;
                    dd.shader = skinnedMeshShader;
                    dd.boneMatrices = BoneMatrixCalculator().calculateFinalSkinMatrices(finalPose);
                    if (finalPose) {
                        delete finalPose;
                    }
                } else {
                    throw std::runtime_error("no pose could be calculated!");
                }

            }

            else if (currentAnimation && !animationPlaying) {
                auto finalPose = BoneMatrixCalculator().calculatePose(currentAnimation, importedMesh->skeleton, currentAnimation->currentDebugTimestamp);
                dd.skinnedDraw = true;
                dd.shader = skinnedMeshShader;
                dd.boneMatrices = BoneMatrixCalculator().calculateFinalSkinMatrices(finalPose);
                if (finalPose) {
                    delete finalPose;
                }
            }
            else if ((currentAnimation || blendedAnimations.size() == 2)  && animationPlaying) {
                animationPlayer->update();
                dd.skinnedDraw = true;
                dd.shader = skinnedMeshShader;
                dd.boneMatrices = animationPlayer->getCurrentBoneMatrices();
            } else {
                dd.shader = staticMeshShader;
            }
            drawMesh(dd);

            // Now draw the wireframe version
            wireframeOn();
            dd.color = glm::vec4(0, 0, 1, 1);
            drawMesh(dd);
            wireframeOff();


        }

        // Draw the skeleton if the mesh has one
        {
            if (importedMesh->skeleton) {
                if (importedMesh->skeleton) {
                    for (auto j: importedMesh->skeleton->joints) {
                        auto finalTransform = j->bindPoseGlobalTransform;
                        if (currentAnimation && !animationPlaying) {
                            // For the bones themselves, we need just their global position.
                            // As we render each bone separately, we do NOT want the final transform, which includes
                            // the inverseBindMatrix.
                            auto pose = BoneMatrixCalculator().calculatePose(currentAnimation, importedMesh->skeleton, currentAnimation->currentDebugTimestamp);
                            finalTransform = BoneMatrixCalculator().calculateGlobalTransform(j, j->currentPoseLocalTransform);
                            if (pose) {
                                delete(pose);
                            }
                            //finalTransform = animationPlayer->calculateInterpolatedGlobalMatrixForJoint(j);
                         } else if (currentAnimation && animationPlaying) {
                             finalTransform = animationPlayer->calculateInterpolatedGlobalMatrixForJoint(j);
                         }

                        MeshDrawData dd;
                        dd.mesh = assetLoader->getMesh("bone_mesh");
                        dd.color = {0.7, 0.1, .1, 1};
                        dd.camera = getMeshViewerCamera();
                        dd.viewPortDimensions = glm::ivec2{skeletalMeshWindowFrameBuffer->texture->bitmap->width, skeletalMeshWindowFrameBuffer->texture->bitmap->height};
                        dd.shader = staticMeshShader;       // We can use the static mesh shader here, as the bones themselves are not skeletal animated.
                        dd.worldTransform = finalTransform;
                        dd.depthTest = false;               // We want to see the bones always, otherwise they would be hidden by the mesh itself.
                        drawMesh(dd);
                    }

                }
            }

            if (importedMesh->ozzSkeleton) {


                {

                    locals_.resize(importedMesh->ozzSkeleton->num_soa_joints());   // We want AoS format (not SoA)

                    // Convert from skeleton.joint_rest_poses() (SoA format)
                    //std::vector<ozz::math::Transform> aos_transforms(num_joints);

                    for (int soa_index = 0, joint_index = 0; soa_index < importedMesh->ozzSkeleton->num_soa_joints(); ++soa_index) {

                        const auto& soa_transform = importedMesh->ozzSkeleton->joint_rest_poses()[soa_index];
                        locals_[soa_index] = soa_transform;

                        // Maybe we don't need this ... as we can (and must already put SoA transforms into our locals
                        // for (int element_index = 0; element_index < 4 && joint_index < num_joints; ++element_index, ++joint_index) {
                        //     auto aosTransform = convertSoaToAos(soa_transform, element_index);
                        //     locals_[joint_index] = aosTransform;
                        // }
                    }
                }

                const int num_joints = importedMesh->ozzSkeleton->num_joints();
                models_.resize(num_joints);

                ozz::animation::LocalToModelJob ltm_job;
                ltm_job.skeleton = importedMesh->ozzSkeleton.get();
                ltm_job.input = ozz::make_span(locals_);
                ltm_job.output = ozz::make_span(models_);
                if (!ltm_job.Run()) {
                    throw new std::runtime_error("could not load ozz model");
                }


                for (auto jointIndex = 0; jointIndex < importedMesh->ozzSkeleton->num_joints(); jointIndex++) {

                        MeshDrawData dd;
                        dd.mesh = assetLoader->getMesh("bone_mesh");
                        dd.color = {0.7, 0.1, .1, 1};
                        dd.camera = getMeshViewerCamera();
                        dd.viewPortDimensions = glm::ivec2{skeletalMeshWindowFrameBuffer->texture->bitmap->width, skeletalMeshWindowFrameBuffer->texture->bitmap->height};
                        dd.shader = staticMeshShader;       // We can use the static mesh shader here, as the bones themselves are not skeletal animated.
                        glm::mat4 modelTransform;
                        std::memcpy(&modelTransform, &ltm_job.output[jointIndex], sizeof(glm::mat4));
                        dd.worldTransform = modelTransform;
                        dd.depthTest = false;               // We want to see the bones always, otherwise they would be hidden by the mesh itself.
                        drawMesh(dd);
                    }

                }

        }

        // Rest to normal viewport:
        //glViewport(0, 0, skeletalMeshWindowFrameBuffer->texture->bitmap->width, skeletalMeshWindowFrameBuffer->texture->bitmap->height);
        glViewport(0, 0, scaled_width, scaled_height);

        // Activate main framebuffer again:
        activateFrameBuffer(nullptr);

        // We render a 2-column table, with the 3D window on the left, and another table with
        // detailed skeleton information on the right.
        if (ImGui::BeginTable("TwoColumnTable", 2)) {

            ImGui::TableNextColumn();
            // Now we are finished rendering our 3D scene into our framebuffer.
            // Next we present the texture of the FBO as an image in ImGUI.
            ImGui::Image((ImTextureID)(intptr_t)(skeletalMeshWindowFrameBuffer->texture->handle),
                             {(float) skeletalMeshWindowFrameBuffer->texture->bitmap->width / 2,
                              (float) skeletalMeshWindowFrameBuffer->texture->bitmap->height / 2},
                             {0, 1}, {1, 0});

            ImGui::TableNextColumn();
            ImGui::Text("Joint infos");

            if (importedMesh->skeleton) {
                if (ImGui::BeginTable("bone_detail_table", 4)) {
                    // Set up headers for the table (optional)
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("localTransform", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("offsetMatrix", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Parent");
                    ImGui::TableHeadersRow();


                    for (auto j: importedMesh->skeleton->joints) {

                        ImGui::TableNextRow();

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", j->name.c_str());

                        ImGui::TableNextColumn();
                        float x = j->currentPoseLocalTransform[3][0]; // Translation x
                        float y = j->currentPoseLocalTransform[3][1]; // Translation y
                        float z = j->currentPoseLocalTransform[3][2]; // Translation z
                        ImGui::Text("%f/%f/%f", x, y, z);

                        ImGui::TableNextColumn();
                        float ibx = j->inverseBindMatrix[3][0]; // Translation x
                        float iby = j->inverseBindMatrix[3][1]; // Translation y
                        float ibz = j->inverseBindMatrix[3][2]; // Translation z
                        ImGui::Text("%f/%f/%f", ibx, iby, ibz);

                        ImGui::TableNextColumn();
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



            // Render current animation details:
            if (currentAnimation ) {
                int count = 0;
                std::string lastJointName = "";

                ImGui::Text("# Frames: %d", currentAnimation->frames);
                if (ImGui::BeginTable("anim_detail_table_rotation", 4)) {
                    ImGui::TableSetupColumn("#");
                    ImGui::TableSetupColumn("Time");
                    ImGui::TableSetupColumn("Joint");
                    ImGui::TableSetupColumn("Rotation", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    for (auto sample: currentAnimation->findSamples(SampleType::rotation)) {
                            ImGui::TableNextRow();

                            if (sample->jointName != lastJointName) {
                                count = 0;
                            }
                            ImGui::TableNextColumn();
                            ImGui::Text("%d ", count++);

                            ImGui::TableNextColumn();
                            ImGui::Text("%f ", sample->time);

                            ImGui::TableNextColumn();
                            ImGui::Text("%s", sample->jointName.c_str());

                            ImGui::TableNextColumn();
                            ImGui::Text("%f/%f/%f", sample->rotation.x, sample->rotation.y, sample->rotation.z);

                            lastJointName = sample->jointName;


                    }

                    ImGui::EndTable();
                }

                if (ImGui::BeginTable("anim_detail_table_translation", 4)) {
                    ImGui::TableSetupColumn("#");
                    ImGui::TableSetupColumn("Time");
                    ImGui::TableSetupColumn("Joint");
                    ImGui::TableSetupColumn("Translation", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();
                    for (auto sample: currentAnimation->findSamples(SampleType::translation)) {
                        ImGui::TableNextRow();

                        if (sample->jointName != lastJointName) {
                            count = 0;
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text("%d ", count++);

                        ImGui::TableNextColumn();
                        ImGui::Text("%f ", sample->time);

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", sample->jointName.c_str());

                        ImGui::TableNextColumn();
                        ImGui::Text("%f/%f/%f", sample->translation.x, sample->translation.y, sample->translation.z);

                        lastJointName = sample->jointName;

                    }
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


    std::string Editor::selectFolderUsingIFileDialog()
    {
        std::wstring folder = L"";
        // Initialize COM.
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr)) {
            IFileDialog *pfd = nullptr;
            // Create the File Open Dialog object.
            hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
            if (SUCCEEDED(hr)) {
                // Set the options to pick folders.
                DWORD dwFlags;
                pfd->GetOptions(&dwFlags);
                pfd->SetOptions(dwFlags | FOS_PICKFOLDERS);

                // Show the dialog.
                hr = pfd->Show(NULL);
                if (SUCCEEDED(hr)) {
                    IShellItem *psiResult = nullptr;
                    hr = pfd->GetResult(&psiResult);
                    if (SUCCEEDED(hr)) {
                        PWSTR pszFilePath = nullptr;
                        hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                        if (SUCCEEDED(hr)) {
                            std::wcout << L"Selected folder: " << pszFilePath << std::endl;
                            folder = pszFilePath;
                            CoTaskMemFree(pszFilePath);
                        }
                        psiResult->Release();
                    }
                }
                pfd->Release();
            }
            CoUninitialize();
        }

        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(folder);
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

            if (ImGui::MenuItem("Select skeleton base folder")) {
                skeletonBaseName = selectFolderUsingIFileDialog();
            }

            if (ImGui::MenuItem("Import Mesh")) {
                if (skeletonBaseName.empty()) {
                    skeletonBaseName = selectFolderUsingIFileDialog();
                }
                doImportMeshAction();
            }

            if (!lastImportedMeshFileName.empty()) {
                if (ImGui::MenuItem("Reimport last mesh")) {
                    if (importedMesh) {
                        delete (importedMesh);
                    }
                    importedMesh = AssimpMeshImporter().importMesh(lastImportedMeshFileName, skeletonBaseName);
                }
            }

            if (ImGui::MenuItem("Load Texture")) {
                lastImporteTextureFileName = showFileDialog("All\0*.*\0png\0*.png\0jpg\0*.jpg\0bmp\0*.bmp");
                if (!lastImporteTextureFileName.empty()) {
                    importedTexture = createTextureFromFile(lastImporteTextureFileName);
                }
            }

            if (ImGui::MenuItem("Import Animation")) {
                auto fileName = showFileDialog("All\0*.*\0fbx\0*.fbx\0gltf\0*.glb");
                if (!fileName.empty()) {
                    auto importedAnims = AssimpMeshImporter().importAnimations(fileName);
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
            importedMesh = AssimpMeshImporter().importMesh(lastImportedMeshFileName, skeletonBaseName);
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

        //drawAnimationOverview();

        renderMeshViewerExt();


    }

    void Editor::drawTopToolbar() {
        ImGui::Begin("Toolbar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowPos(ImVec2(0, 30));     // Position at the top
        float toolbarHeight = 40;
        if (ImGui::ImageButton((ImTextureID)(intptr_t) assetLoader->getTexture("button_play")->handle, {16, 16})) {
            printf("play button pressed\n");
        }
        ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, toolbarHeight));  // Full window width, height of 50
        ImGui::End();
    }

    void Editor::drawAnimationOverview() {

        Camera _uiCamera;
        _uiCamera.location = {0, 0, 1};
        _uiCamera.lookAtTarget = {0, 0, -1};
        _uiCamera.type = CameraType::Ortho;
        bindTexture(nullptr);
        bindCamera(&_uiCamera);
        location({0, 0, -0.5});
        scale({100, 64, 1});
        forceShader(gradientShader);
        drawPlane();

        // static auto texture = createEmptyTexture(1024, 512);
        // static auto pb = new PixelBuffer(texture);
        // pb->clear({10, 10, 10, 255});

        // Line tests
        {

            // Line with single points
            // for (int x = 0; x < 512; x++) {
            //     pb->drawPoint({x, 100}, {255, 0, 0, 255});
            // }

            // pb->drawLine({10, 10}, {100, 15}, {255, 100, 10, 255});
            // pb->drawLine({10, 10}, {30, 80}, {210, 100, 10, 255});
            //
            // // Now mirrored one, right to left effectively:
            // pb->drawLine({30, 80}, {10, 160}, {140, 100, 10, 255});
            // pb->drawLine({130, 200}, {150, 70}, {140, 100, 10, 255});
            // pb->drawLine({200, 10}, {100, 15}, {255, 100, 10, 255});
        }

        // Draw bg grid:
        // int gridSize= 40;
        // for (int i = 0; i < (1024/gridSize) + 1; i++) {
        //     pb->drawLine({(i*gridSize), 0}, {i*gridSize, 511}, {30, 30, 30, 255});
        // }

        // Frame
        //pb->drawRect({0, 511}, {1023, 0}, {60, 60, 60, 255});
        // pb->drawLine({0, 0}, {0, 511}, {60, 60, 60, 255});
        // pb->drawLine({0, 511}, {1023, 511}, {60, 60, 60, 255});
        // pb->drawLine({1023, 511}, {1023, 0}, {60, 60, 60, 255});
        // pb->drawLine({0, 0}, {1023, 0}, {60, 60, 60, 255});

        //updateTexture(1024, 512, texture);

        // ImGui::Image(reinterpret_cast<ImTextureID>(texture->handle),
        //                     {(float) texture->bitmap->width ,
        //                      (float) texture->bitmap->height },
        //                     {0, 1}, {1, 0});
    }

    void Editor::drawAnimationTimeline() {

#ifdef _IMGUI_TIMELINE
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



        // Draw line numbers as text
        for (int x = 0; x < 20; x++) {
            draw_list->AddText({3 + 64.0f * (float) x + beginTimeLineX, win_pos.y + (toolbarHeight - 22)},
                               IM_COL32(100, 120, 10, 150), std::to_string(x).c_str());
        }

        ImGui::EndChild();
        // Close the toolbar window
#endif

        ImGui::Text("Animations");
        ImGui::Text("Current Active animation: %s",
                    (currentAnimation != nullptr ? currentAnimation->name.c_str() : "--"));


        if (ImGui::Button("Play")) {
            animationPlaying = true;
            animationPlayer->play(animationLooped);
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            animationPlaying = false;
        }
        ImGui::SameLine();
        ImGui::Checkbox("Loop", &animationLooped);


        // Merge all animations we have together here (also the ones without a mesh..):
        // We might want to see those as a skeleton only?!
        std::vector<Animation*> allAnims;
        for (auto anim : importedAnimations | std::views::values) {
            allAnims.push_back(anim);
        }
        if (importedMesh) {
            for (auto anim : importedMesh->animations) {
                allAnims.push_back(anim);
            }
        }

        if (!allAnims.empty()) {
            auto animations = allAnims;
            int sampleOffsetY = 0;
            int animIndex = 0;
            for (auto anim: animations) {
                std::vector<std::string> joints;
                // Iterate over the map and collect keys
                for (const auto &sample: anim->findSamples(SampleType::rotation)) {
                    joints.push_back(sample->jointName);
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
                //ImGui::Text("Current frame: %d", currentAnimationFrame);
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
                if (ImGui::Button(std::string("Deactivate##Deactivate" + anim->name).c_str())) {
                    animationPlayer->switchAnimation(nullptr);
                    animationPlayer->stop();

                    currentAnimation = nullptr;
                    currentAnimationFrame = 0;
                    importedMesh->skeleton->resetToBindPose();
                }

                ImGui::SameLine();
                ImGui::Text("TS:");
                ImGui::SameLine();
                ImGui::PushItemWidth(50);
                std::string animUID = "##ts" + std::to_string(animIndex);
                ImGui::InputFloat(animUID.c_str(), &anim->currentDebugTimestamp);
                ImGui::SameLine();
                ImGui::PopItemWidth();
                ImGui::Text("Mask:");
                ImGui::PushItemWidth(50);
                ImGui::SameLine();
                std::string maskUID = "##Mask" + std::to_string(animIndex);
                static char maskBuf[50];
                ImGui::InputText(maskUID.c_str(), maskBuf, sizeof(maskBuf));
                ImGui::PopItemWidth();
                ImGui::SameLine();
                std::string lblBlendAdd = "Add##" + anim->name;
                if (ImGui::Button(lblBlendAdd.c_str())) {
                    blendedAnimations.emplace(anim);
                    if (blendedAnimations.size() == 2) {
                        PerBoneBlendData* perBoneBlendData = new PerBoneBlendData();
                        perBoneBlendData->addNamedBoneWeight("upperarm.r", 1);
                        perBoneBlendData->addNamedBoneWeight("lowerarm.r", 1);
                        perBoneBlendData->addNamedBoneWeight("hand.r", 1);
                        perBoneBlendData->addNamedBoneWeight("upperarm.l", 1);
                        perBoneBlendData->addNamedBoneWeight("lowerarm.l", 1);
                        perBoneBlendData->addNamedBoneWeight("hand.l", 1);
                        std::vector<Animation*> animList = std::vector(blendedAnimations.begin(), blendedAnimations.end());
                        auto ab = new AnimationBlender(animList[0], animList[1], perBoneBlendData );
                        animationPlayer->setAnimationBlender(ab);
                    }
                }
                ImGui::SameLine();
                std::string lblBlendRemove = "Remove##" + anim->name;
                if (ImGui::Button(lblBlendRemove.c_str())) {
                    blendedAnimations.erase(anim);
                }

                ImGui::SameLine();
                std::string lblApplyMask = "Apply Mask##" + anim->name;
                if (ImGui::Button(lblApplyMask.c_str())) {
                    static auto jm1 = new JointMask(importedMesh->skeleton, MaskType::Hierarchy, HierarchyDirection::down, "neck");
                    anim->applyJointMask(jm1);

                    static auto jm2 = new JointMask(importedMesh->skeleton, MaskType::List);
                    jm2->addJointByName("upperarm.r");
                    jm2->addJointByName("lowerarm.l");
                    jm2->addJointByName("hand.l");
                    jm2->addJointByName("Bone");
                    //anim->applyJointMask(jm2);

                }
                animIndex++;
            }
        }
        if (!blendedAnimations.empty()) {
            ImGui::Dummy({0, 30});
            ImGui::TextColored({1, 0, 0.5, 1}, "Blending details");
            ImGui::Dummy({0, 5});
            ImGui::Text("Blend weight:");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::InputFloat("##BlendWeight", &blendWeight);
            ImGui::Text("Blend timestamp:");
            ImGui::SameLine();
            ImGui::InputFloat("##BlendTimesamp", &blendTimestamp);
            ImGui::PopItemWidth();
            if (ImGui::BeginTable("blend_detail_table", 3)) {
                // Set up headers for the table (optional)
                ImGui::TableSetupColumn("Animation Name");
                ImGui::TableHeadersRow();

                auto blendedAnimsList = std::vector(blendedAnimations.begin(), blendedAnimations.end());
                for (auto banim: blendedAnimations) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", banim->name.c_str());
                }
                ImGui::EndTable();
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
        animationPlayer = new AnimationPlayer();
        staticMeshShader = new Shader();
        staticMeshShader->initFromFiles("../src/engine/editor/assets/shaders/colored_mesh.vert", "../src/engine/editor/assets/shaders/colored_mesh.frag");
        skinnedMeshShader = new Shader();
        skinnedMeshShader->initFromFiles("../src/engine/editor/assets/shaders/skinned_mesh.vert", "../src/engine/editor/assets/shaders/colored_mesh.frag");
        gradientShader = new Shader();
        gradientShader->initFromFiles("../src/engine/editor/assets/shaders/colored_mesh.vert", "../src/engine/editor/assets/shaders/colored_mesh.frag");
    }

    Editor::~Editor() {
        locals_.clear();
        models_.clear();
        if (importedMesh && importedMesh->ozzSkeleton) {
            importedMesh->ozzSkeleton.reset();
        }


    }

    void EditorGame::init() {
        DefaultGame::init();

        editor = new Editor();
    }

    EditorGame::~EditorGame() {
        if (editor) delete editor;
    }

    void EditorGame::update() {

        editor->update();
    }

    void EditorGame::render() {
        editor->renderImGui();
        renderFPS();
    }

    bool EditorGame::shouldStillRun() {
        return true;
    }
}

//DefaultGame* getGame() {
//    return nullptr;
//}