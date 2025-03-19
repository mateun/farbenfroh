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

    void addNode(SceneNode* node);
    void update();

    void activateDebugFlyCam(bool value);
    Camera* getDebugFlyCam();
    SceneNode* findActiveCameraNode();

    std::vector<Light *> getLightsOfType(LightType type);

    void flattenNodes(const std::vector<SceneNode*>& sourceNodeTree, std::vector<SceneNode*>& targetList);

    void render();

private:
    std::vector<Light*> pointLights;
    Camera* uiCamera = nullptr;
    Camera* debugFlyCam = nullptr;
    CameraMover * flyCamMover = nullptr;
    bool debugFlyCamActive = false;

    std::vector<SceneNode*> meshNodes;
    std::vector<SceneNode*> textNodes;
    std::vector<SceneNode*> cameraNodes;
    std::vector<SceneNode*> particleSystemNodes;
    std::vector<SceneNode*> directionalLightNodes;
    std::vector<SceneNode*> pointLightNodes;
    std::vector<SceneNode*> spotLightNodes;

    Texture* rayTraceWorldPosTexture = nullptr;

    FrameBuffer* raytracedShadowPositionFBO = nullptr;
    FrameBuffer * fullScreenFBO = nullptr;


    Shader * worldPosShader = nullptr;
    Shader * shadowMapShader = nullptr;
    Shader * quadShader = nullptr;

    Mesh * quadMesh = nullptr;

};


#endif //SCENE_H
