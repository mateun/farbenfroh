//
// Created by mgrus on 19.03.2025.
//

#include "Scene.h"


Scene::Scene() {
    rayTraceWorldPosTexture = createEmptyFloatTexture(scaled_width, scaled_height);
    raytracedShadowPositionFBO =  createFrameBufferWithTexture(scaled_width, scaled_height, rayTraceWorldPosTexture);
    worldPosShader = new Shader();
    worldPosShader->initFromFiles("../assets/shaders/world_pos.vert", "../assets/shaders/world_pos.frag");
    shadowMapShader = new Shader();
    shadowMapShader->initFromFiles("../assets/shaders/shadow_map.vert", "../assets/shaders/shadow_map.frag");
    quadShader = new Shader();
    quadShader->initFromFiles("../src/engine/fx/shaders/quad.vert", "../src/engine/fx/shaders/quad.frag");
    quadMesh = createQuadMesh(PlanePivot::center);

    debugFlyCam = new Camera();
    debugFlyCam->type = CameraType::Perspective;
    debugFlyCam->location = glm::vec3(10, 10, -5);
    debugFlyCam->lookAtTarget = glm::vec3(0, 0, 0);
    debugFlyCam->updateNearFar(0.1, 400);
    flyCamMover = new CameraMover(debugFlyCam);
    fullScreenFBO = createFrameBuffer(scaled_width, scaled_height, true, false);




}

Scene::~Scene() {
}

std::vector<Light *> Scene::getLightsOfType(LightType type) {
    std::vector<Light*> ls;
    if (type == LightType::Directional) {
        for (auto ln : directionalLightNodes) {
            ls.push_back(ln->light);
        }
    }
    else if (type == LightType::Point) {
        for (auto ln : pointLightNodes) {
            ls.push_back(ln->light);
        }
    }
    else if (type == LightType::Spot) {
        for (auto ln : spotLightNodes) {
            ls.push_back(ln->light);
        }
    }

    return ls;

}

/**
* Returns the first active camera it finds.
*/
SceneNode* Scene::findActiveCameraNode() {
    for (auto camNode : cameraNodes) {
        if (camNode->isActive()) {
            return camNode;
        }
    }
    return nullptr;
}


void Scene::flattenNodes(const std::vector<SceneNode*>& sourceNodeTree, std::vector<SceneNode*>& targetList) {
    for (auto n : sourceNodeTree) {
        targetList.push_back(n);
        flattenNodes(n->children, targetList);
    }
}



void Scene::setUICamera(Camera *cam) {
    this->uiCamera = cam;
}


void Scene::addNode(SceneNode *node) {
    if (node->_type == SceneNodeType::Mesh) {
        meshNodes.push_back(node);
    }

    if (node->_type == SceneNodeType::ParticleSystem) {
        particleSystemNodes.push_back(node);
    }

    if (node->_type == SceneNodeType::Text) {
        textNodes.push_back(node);
    }

    if (node->_type == SceneNodeType::Camera) {
        cameraNodes.push_back(node);
    }
    if (node->_type == SceneNodeType::Light) {
        if (node->light->type == LightType::Directional) {
            directionalLightNodes.push_back(node);
        }
        else if (node->light->type == LightType::Point) {
            pointLightNodes.push_back(node);
        }
        else if (node->light->type == LightType::Spot) {
            spotLightNodes.push_back(node);
        }
    }
}

void Scene::update() {
    if (debugFlyCamActive) {
        flyCamMover->update();
    }

    for (auto ln : pointLightNodes) {
        ln->light->location = ln->getLocation();
    }

    for (auto ps: particleSystemNodes) {
        if (!ps->isActive()) {
            continue;
        }
        ps->particleSystem->update();
    }
}

void Scene::activateDebugFlyCam(bool value) {
    debugFlyCamActive = value;
    if (value == true) {
        for (auto dl : directionalLightNodes) {
            dl->light->initFrustumDebugging(findActiveCameraNode()->camera);
            dl->light->updateFrustumDebugging(findActiveCameraNode()->camera);
        }

    }

}

Camera * Scene::getDebugFlyCam() {
    return debugFlyCam;
}




// MeshDrawData Scene::renderNodeRecursively(SceneNode* root, Camera* camera, Light* light) {
//     MeshDrawData dd;
//     dd.camera = camera;
//     dd.location = root->getHierarchicalWorldLocation(root->getLocation());
//     dd.scale = root->_scale;
//     dd.shader = shadowMapShader;
//     dd.rotationEulers = root->_rotationInDeg;
//     dd.mesh = root->mesh;
//     dd.viewPortDimensions = {light->shadowMapFBO->texture->bitmap->width, light->shadowMapFBO->texture->bitmap->height};
//     dd.directionalLights.push_back(light);
//     if (root->skinnedMesh) {
//         dd.skinnedDraw = root->skinnedMesh;
//         dd.boneMatrices = root->boneMatrices();
//     }
//
//     // TODO how to handle the recursive handling of the
//     return dd;
//
//     for (auto child : root->children) {
//         renderNodeRecursively(child, camera, light);
//     }
// }

void Scene::render() {
    // 1. Shadow pass
    // Check all lights which cast a shadow
    // These need to be used to for the shadowpass.
    // So we need to render all meshes into the shadowmap.
    // We need to this for every light and every mesh.


#ifdef _DEBUG_RAYTRACING
    // Experimental: raytraced shadowing.
    // First render all world positions into a framebuffer.
    // We use this later to raycast to any light and write the
    // information into another dedicated shadow texture.
    // Both textures are already in final camera render space,
    // so there is no loss in shadow quality.
    {
        auto cameraNode = findActiveCameraNode();
        glBindFramebuffer(GL_FRAMEBUFFER, raytracedShadowPositionFBO->handle);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        for (auto m : meshNodes) {
            if (!m->isActive()) {
                continue;
            }
            // Build the actual render commands for each node and
            // render it into the shadow map.
            MeshDrawData mdd;
            mdd.location = m->_location;
            mdd.scale = m->_scale;
            mdd.rotationEulers = m->_rotationInDeg;
            mdd.mesh = m->mesh;
            //mdd.texture = m->texture;
            //mdd.normalMap = m->normalMap;
            mdd.shader = worldPosShader;
            mdd.camera = cameraNode->getCamera();
            //mdd.uvScale = m->uvScale;
            //mdd.color = m->foregroundColor;
            //mdd.directionalLights = getLightsOfType(LightType::Directional);
            //mdd.pointLights = getLightsOfType(LightType::Point);
            //mdd.spotLights = getLightsOfType(LightType::Spot);

            if (m->skinnedMesh) {
                mdd.skinnedDraw = m->skinnedMesh;
                mdd.boneMatrices = m->boneMatrices();
            }

            drawMesh(mdd);

        }
    }

    // Debug the created texture:

    float* data = new float[scaled_width * scaled_height * 3];
    glReadPixels(0, 0, scaled_width, scaled_height, GL_RGB, GL_FLOAT, data);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    GL_ERROR_EXIT(12999)
    // 'data' now holds the framebuffer's pixel data

    for (int x = 0; x < scaled_width; x++) {
        for (int y = 0; y < scaled_height; y++) {
            int offset = (x*3) + (y*scaled_width *3);
            float posx = data[offset];
            float posy = data[offset+1];
            float posz = data[offset+2];

            if (posx != 0) {
                printf("found posx for %d/%d\n", x, y);
            }
        }
    }
#endif

    // End experimental

    auto cameraNode = findActiveCameraNode();
    if (!cameraNode) {
        throw std::runtime_error("Could not find active camera node.");
    }

    auto activeCamera = debugFlyCamActive ? debugFlyCam : cameraNode->getCamera();

    std::vector<SceneNode*> flatMeshNodes;
    flattenNodes(meshNodes, flatMeshNodes);

    // First, our directional lights
    {

        for (auto l: directionalLightNodes) {
            glBindFramebuffer(GL_FRAMEBUFFER, l->light->shadowMapFBO->handle);
            glClear(GL_DEPTH_BUFFER_BIT);

            for (auto m : flatMeshNodes) {
                if (!m->isActive() || !m->getMeshData().castShadow) {
                    continue;
                }
                MeshDrawData dd;
                dd.camera = cameraNode->getCamera();
                dd.location = m->getHierarchicalWorldLocation(m->_location);
                dd.scale = m->_scale;
                dd.color == m->foregroundColor;
                dd.shader = shadowMapShader;
                auto worldOrientation = m->getWorldOrientation();
                dd.rotationEulers = degrees(eulerAngles(worldOrientation));
                dd.mesh = m->mesh;
                dd.viewPortDimensions = {l->light->shadowMapFBO->texture->bitmap->width, l->light->shadowMapFBO->texture->bitmap->height};
                dd.directionalLights.push_back(l->light);
                if (m->skinnedMesh) {
                     dd.skinnedDraw = m->skinnedMesh;
                     dd.boneMatrices = m->boneMatrices();
                }

                if (debugFlyCamActive) {
                    continue;
                }

                glCullFace(GL_FRONT);
                glPolygonOffset(1, 0.75);
                drawMeshIntoShadowMap(dd, l->light);
                glCullFace(GL_BACK);

            }
        }

    }
#ifdef POINTLIGHT_SHADOWS
    // Next the pointlights
    for (auto lightNode : pointLightNodes) {
        if (!lightNode->light->castsShadow) continue;

        lightNode->light->shadowMapFBO->handle;

        // Build camera out of current light.
        // We need this so the view/projection matrices are built correctly.
        Camera lightCam;
        lightCam.location = lightNode->light->location;
        lightCam.lookAtTarget = lightNode->light->lookAtTarget;
        lightCam.type = CameraType::Perspective;


        bindShadowMapCamera(&lightCam);
        glBindFramebuffer(GL_FRAMEBUFFER, lightNode->light->shadowMapFBO->handle);
        glClear(GL_DEPTH_BUFFER_BIT);

        for (auto m : meshNodes) {
            if (!m->isActive()) {
                continue;
            }
            // Build the actual render commands for each node and
            // render it into the shadow map.
            location(m->_location);
            scale(m->_scale);
            rotation(m->_rotationInDeg);
            bindMesh(m->mesh);

            // TODO
            // if (m->skinnedMesh) {
            //     mdd.skinnedDraw = m->skinnedMesh;
            //     mdd.boneMatrices = m->boneMatrices;
            // }


            drawMeshIntoShadowMap(lightNode->light->shadowMapFBO);
        }
    }

#endif

    // Back to our "normal" framebuffer or to another offscreen buffer in case we have activated post
    // processing effects:
    if (!activeCamera->getPostProcessEffects().empty()) {
        activateFrameBuffer(fullScreenFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, fullScreenFBO->texture->bitmap->width, fullScreenFBO->texture->bitmap->height);
    } else {
        // Render directly to standard framebuffer.
        activateFrameBuffer(nullptr);
        glViewport(0, 0, scaled_width, scaled_height);
    }


    // 2. Render meshes
    for (auto m : flatMeshNodes) {

        if (!m->isActive()) {
            continue;
        }

        // Build the actual render commands for each node and
        // render it into the shadow map.
        MeshDrawData mdd;
        mdd.camera = activeCamera;
        mdd.location = m->getHierarchicalWorldLocation(m->_location);
        mdd.scale = m->_scale;
        auto worldOrientation = m->getWorldOrientation();
        mdd.rotationEulers = degrees(eulerAngles(worldOrientation));

        mdd.mesh = m->mesh;
        mdd.tint = m->meshData.tint;
        mdd.texture = m->texture;
        mdd.normalMap = m->normalMap;
        mdd.shader = m->shader;
        mdd.uvPan = m->uvPan;
        mdd.uvScale = m->uvScale;
        mdd.normalUVScale2 = m->meshData.normalUVScale2;
        mdd.uvScale2 = m->uvScale2;
        mdd.color = m->foregroundColor;
        mdd.directionalLights = getLightsOfType(LightType::Directional);
        mdd.pointLights = getLightsOfType(LightType::Point);
        mdd.spotLights = getLightsOfType(LightType::Spot);

        if (m->skinnedMesh) {
            mdd.skinnedDraw = m->skinnedMesh;
            mdd.boneMatrices = m->boneMatrices();
        }

        // Invoke custom onRender callback functions
        if (m->meshData.onRender) {
            m->meshData.onRender(m->meshData);
        }

        drawMesh(mdd);

    }

    // 3. Render particles
    std::vector<SceneNode*> flatParticleNodes;
    flattenNodes(particleSystemNodes, flatParticleNodes);
    for (auto ps : flatParticleNodes) {
        if (!ps->isActive()) {
            continue;
        }
        ps->particleSystem->render(activeCamera);

    }

    // Postprocessing, if active:
    // First, apply all post effects:
    if (!activeCamera->getPostProcessEffects().empty()) {
        FrameBuffer* currentFB = fullScreenFBO;
        for (auto pp : activeCamera->getPostProcessEffects()) {
            currentFB = pp->apply(currentFB, uiCamera);
        }




        activateFrameBuffer(nullptr);
        MeshDrawData mdd;
        mdd.camera = uiCamera;
        mdd.mesh = quadMesh;
        mdd.shader = quadShader;
        mdd.location = { scaled_width/2, scaled_height/2, -5};
        mdd.scale = { scaled_width, scaled_height, 1};
        mdd.texture = currentFB->texture;
        drawMesh(mdd);


    }

    // Render some debug stuff:
    if (debugFlyCamActive) {
        for (auto dl : directionalLightNodes) {
            dl->light->renderWorldFrustum(debugFlyCam, cameraNode->getCamera());
        }

    }

}
