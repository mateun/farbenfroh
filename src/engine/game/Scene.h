//
// Created by mgrus on 19.03.2025.
//

#ifndef SCENE_H
#define SCENE_H
#include <vector>

class Camera;
class SceneNode;
enum class LightType;
class Light;
class CameraMover;
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
    Camera* getDebugFlyCam() const;
    const SceneNode* findActiveCameraNode() const;

    std::vector<Light*> getLightsOfType(LightType type) const;

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

    std::unique_ptr<Shader> worldPosShader = nullptr;
    std::unique_ptr<Shader> shadowMapShader = nullptr;
    std::unique_ptr<Shader> quadShader = nullptr;

    std::unique_ptr<Mesh> quadMesh = nullptr;

};


#endif //SCENE_H
