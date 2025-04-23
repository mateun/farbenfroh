//
// Created by mgrus on 19.03.2025.
//

#ifndef SCENE_H
#define SCENE_H
#include <memory>
#include <string>
#include <vector>
#include <engine/graphics/CameraMover.h>
#include <engine/lighting/Light.h>

class Camera;
class SceneNode;

class Light;

class Texture;
class FrameBuffer;
class Mesh;
class Shader;

class Scene {
public:
    Scene();
    ~Scene();

    void setUICamera(Camera* cam);

    void addNode(std::shared_ptr<SceneNode> node);
    void update();

    void activateDebugFlyCam(bool value);
    [[nodiscard]] Camera* getDebugFlyCam() const;
    [[nodiscard]] const SceneNode* findActiveCameraNode() const;

    [[nodiscard]] std::vector<Light*> getLightsOfType(LightType type) const;

    void flattenNodes(const std::vector<std::shared_ptr<SceneNode>>& sourceNodeTree, std::vector<SceneNode*>& targetList) const;

    void render() const;

    SceneNode* findNodeById(const std::string& id);
    static SceneNode* findFirstInactive(const std::vector<std::shared_ptr<SceneNode>> & nodeList);

private:
    std::vector<std::shared_ptr<Light>> pointLights;
    std::shared_ptr<Camera> uiCamera = nullptr;
    std::unique_ptr<Camera> debugFlyCam = nullptr;
    std::unique_ptr<CameraMover> flyCamMover = nullptr;
    bool debugFlyCamActive = false;

    std::vector<std::shared_ptr<SceneNode>> _meshNodes;
    std::vector<std::shared_ptr<SceneNode>> _textNodes;
    std::vector<std::shared_ptr<SceneNode>> _cameraNodes;
    std::vector<std::shared_ptr<SceneNode>> _particleSystemNodes;
    std::vector<std::shared_ptr<SceneNode>> _directionalLightNodes;
    std::vector<std::shared_ptr<SceneNode>> _pointLightNodes;
    std::vector<std::shared_ptr<SceneNode>> _spotLightNodes;
    std::vector<std::shared_ptr<SceneNode>> _allNodes;

    std::shared_ptr<Texture> rayTraceWorldPosTexture = nullptr;

    std::shared_ptr<FrameBuffer> raytracedShadowPositionFBO = nullptr;
    std::shared_ptr<FrameBuffer> fullScreenFBO = nullptr;

    std::shared_ptr<Shader> worldPosShader = nullptr;
    std::shared_ptr<Shader> shadowMapShader = nullptr;
    std::shared_ptr<Shader> quadShader = nullptr;

    std::shared_ptr<Mesh> quadMesh = nullptr;

};


#endif //SCENE_H
