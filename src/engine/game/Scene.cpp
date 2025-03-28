//
// Created by mgrus on 19.03.2025.
//

#include "Scene.h"
#include <memory>
#include <engine/fx/ParticleSystem.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/Camera.h>
#include <engine/graphics/Texture.h>
#include <engine/graphics/FrameBuffer.h>
#include <engine/graphics/Geometry.h>
#include <engine/graphics/PlanePivot.h>
#include <engine/lighting/Light.h>
#include <engine/game/SceneNode.h>
#include <engine/graphics/ErrorHandling.h>
#include <engine/graphics/Renderer.h>
#include <engine/fx/PostProcessEffect.h>
#include <engine/graphics/StatefulRenderer.h>

Scene::Scene() {
    rayTraceWorldPosTexture = Texture::createEmptyFloatTexture(getApplication()->scaled_width(), getApplication()->scaled_height());
    raytracedShadowPositionFBO =  std::make_unique<FrameBuffer>(rayTraceWorldPosTexture);
    worldPosShader = std::make_unique<Shader>();
    worldPosShader->initFromFiles("../assets/shaders/world_pos.vert", "../assets/shaders/world_pos.frag");
    shadowMapShader = std::make_unique<Shader>();
    shadowMapShader->initFromFiles("../assets/shaders/shadow_map.vert", "../assets/shaders/shadow_map.frag");
    quadShader = std::make_unique<Shader>();
    quadShader->initFromFiles("../src/engine/fx/shaders/quad.vert", "../src/engine/fx/shaders/quad.frag");
    quadMesh = gru::Geometry::createQuadMesh(PlanePivot::center);


    debugFlyCam = std::make_unique<Camera>();
    debugFlyCam->type = CameraType::Perspective;
    debugFlyCam->location = glm::vec3(10, 10, -5);
    debugFlyCam->lookAtTarget = glm::vec3(0, 0, 0);
    debugFlyCam->updateNearFar(0.1, 400);
    flyCamMover = std::make_unique<CameraMover>(debugFlyCam.get());
    fullScreenFBO = std::make_shared<FrameBuffer>(getApplication()->scaled_width(), getApplication()->scaled_height(), true, false);


}

Scene::~Scene() {
}

std::vector<Light *> Scene::getLightsOfType(LightType type) const {
    std::vector<Light*> ls;
    if (type == LightType::Directional) {
        for (auto ln : _directionalLightNodes) {
            ls.push_back(ln->light);
        }
    }
    else if (type == LightType::Point) {
        for (auto ln : _pointLightNodes) {
            ls.push_back(ln->light);
        }
    }
    else if (type == LightType::Spot) {
        for (auto ln : _spotLightNodes) {
            ls.push_back(ln->light);
        }
    }

    return ls;

}

/**
* Returns the first active camera it finds.
*/
const SceneNode* Scene::findActiveCameraNode() const {
    for (auto camNode : _cameraNodes) {
        if (camNode->isActive()) {
            return camNode.get();
        }
    }
    return nullptr;
}


void Scene::flattenNodes(const std::vector<std::shared_ptr<SceneNode>>& sourceNodeTree, std::vector<SceneNode*>& targetList) const {
    for (auto n : sourceNodeTree) {
        targetList.push_back(n.get());
        flattenNodes(n->children, targetList);
    }
}



void Scene::setUICamera(Camera *cam) {
    this->uiCamera = std::shared_ptr<Camera>(cam);
}


void Scene::addNode(std::shared_ptr<SceneNode> node) {
    const std::shared_ptr sharedNode = std::move(node);
    _allNodes.push_back(sharedNode);

    if (sharedNode->_type == SceneNodeType::Mesh) {
        _meshNodes.push_back(sharedNode);
    }

    if (sharedNode->_type == SceneNodeType::ParticleSystem) {
        _particleSystemNodes.push_back(sharedNode);
    }

    if (sharedNode->_type == SceneNodeType::Text) {
        _textNodes.push_back(sharedNode);
    }

    if (sharedNode->_type == SceneNodeType::Camera) {
        _cameraNodes.push_back(sharedNode);
    }
    if (sharedNode->_type == SceneNodeType::Light) {
        if (sharedNode->light->type == LightType::Directional) {
            _directionalLightNodes.push_back(sharedNode);
        }
        else if (sharedNode->light->type == LightType::Point) {
            _pointLightNodes.push_back(sharedNode);
        }
        else if (sharedNode->light->type == LightType::Spot) {
            _spotLightNodes.push_back(sharedNode);
        }
    }
}

void Scene::update() {
    if (debugFlyCamActive) {
        flyCamMover->update();
    }

    std::vector<SceneNode*> flatAll;
    flattenNodes(_allNodes, flatAll);
    for (auto n : flatAll) {
        if (n->isActive()) {
            n->update();
        }
    }

    for (auto ln : _pointLightNodes) {
        ln->light->location = ln->getLocation();
    }


}

void Scene::activateDebugFlyCam(bool value) {
    debugFlyCamActive = value;
    if (value == true) {
        for (auto dl : _directionalLightNodes) {
            dl->light->initFrustumDebugging(findActiveCameraNode()->camera);
            dl->light->updateFrustumDebugging(findActiveCameraNode()->camera);
        }

    }

}

Camera * Scene::getDebugFlyCam() const {
    return debugFlyCam.get();
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

void Scene::render() const {
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

    auto activeCamera = debugFlyCamActive ? debugFlyCam.get() : cameraNode->getCamera();

    std::vector<SceneNode*> flatMeshNodes;
    flattenNodes(_meshNodes, flatMeshNodes);

    // First, our directional lights
    {

        for (auto l: _directionalLightNodes) {
            l->light->shadowMapFBO->bind();
            l->light->shadowMapFBO->clearDepth();


            for (auto m : flatMeshNodes) {
                if (!m->isActive() || !m->getMeshData().castShadow || m->_type == SceneNodeType::ParticleSystem) {
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
                dd.mesh = std::shared_ptr<Mesh>(m->mesh);
                dd.viewPortDimensions = {l->light->shadowMapFBO->width(), l->light->shadowMapFBO->height()};
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
                StatefulRenderer::drawMeshIntoShadowMap(dd, l->light);
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
        StatefulRenderer::activateFrameBuffer(fullScreenFBO.get());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, fullScreenFBO->width(), fullScreenFBO->height());
    } else {
        // Render directly to standard framebuffer.
        StatefulRenderer::activateFrameBuffer(nullptr);
        glViewport(0, 0, getApplication()->scaled_width(), getApplication()->scaled_height());
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

        mdd.mesh = std::shared_ptr<Mesh>(m->mesh);
        mdd.tint = m->meshData.tint;
        mdd.texture = m->texture;
        mdd.normalMap = m->normalMap;
        mdd.shader = std::shared_ptr<Shader>(m->shader);
        mdd.uvPan = m->meshData.uvPan;
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

        GL_ERROR_EXIT(44556611);

        // TODO remove this special treatment of particle system nodes?!
        if (m->_type == SceneNodeType::ParticleSystem) {
            m->particleSystem->render(activeCamera);
        } else {
            Renderer::drawMesh(mdd);
        }


    }






    // Postprocessing, if active:
    // First, apply all post effects:
    if (!activeCamera->getPostProcessEffects().empty()) {
        const FrameBuffer* currentFB = fullScreenFBO.get();
        for (auto pp : activeCamera->getPostProcessEffects()) {
            currentFB = pp->apply(currentFB, uiCamera.get());
        }

        StatefulRenderer::activateFrameBuffer(nullptr);
        MeshDrawData mdd;
        mdd.camera = uiCamera.get();
        mdd.mesh = quadMesh;
        mdd.shader = quadShader;
        mdd.location = { getApplication()->scaled_width()/2, getApplication()->scaled_height()/2, -5};
        mdd.scale = { getApplication()->scaled_width(), getApplication()->scaled_height(), 1};
        mdd.texture = currentFB->texture().get();
        Renderer::drawMesh(mdd);


    }

    // Render some debug stuff:
    if (debugFlyCamActive) {
        for (auto dl : _directionalLightNodes) {
            dl->light->renderWorldFrustum(debugFlyCam.get(), cameraNode->getCamera());
        }

    }

}

SceneNode * Scene::findNodeById(const std::string &id) {
    for (auto n : _allNodes) {
        if (n->getId() == id) {
            return n.get();
        }
    }
    return nullptr;
}

SceneNode * Scene::findFirstInactive(const std::vector<std::shared_ptr<SceneNode>>& nodeList) {
        for (auto& n : nodeList) {
            if (!n->isActive()) {
                return n.get();
            }
        }
        return nullptr;
}
