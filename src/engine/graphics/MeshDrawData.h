//
// Created by mgrus on 25.03.2025.
//

#ifndef MESHDRAWDATA_H
#define MESHDRAWDATA_H
#include <glm\glm.hpp>
#include <vector>
#include <optional>
#include <functional>
#include <engine/graphics/ShaderParameter.h>
#include <engine/graphics/Texture.h>

class Mesh;
class Shader;
class Camera;

class Light;


/**
* This holds everything we need to issue a physical draw call.
* A lot ... but what can you do.
*/
struct MeshDrawData {
    glm::vec3 location = {0, 0, 0};
    glm::vec3 scale = {1, 1, 1};
    glm::vec3 rotationEulers = {0, 0, 0};
    std::vector<glm::mat4> boneMatrices;
    std::optional<glm::mat4> worldTransform;
    Mesh* mesh = nullptr;
    Shader* shader = nullptr;

    std::optional<glm::mat4> rotationMatrix;

    // This is the camera through which a mesh is rendered.
    // Can also be used during shadow map (depth) rendering to allow for the shadow map
    // frustum to be fit inside the view frustum tightly.
    const Camera* camera = nullptr;

    // This is the "camera" which renders the mesh into the shadow depth buffer.
    // We want to have both here as for the shadow map rendering we may try to fit the shadow map frustum
    // into the view frustum (provided by the actual viewing camera).
    Camera* shadowMapCamera = nullptr;

    Texture* texture = nullptr;
    Texture* normalMap = nullptr;
    glm::vec4 color = {1, 0, 1, 1}; // Nice magenta if we have no texture set.
    glm::vec4 tint = {1, 1,1,1};
    std::vector<Light*> directionalLights;
    std::vector<Light*> pointLights;
    std::vector<Light*> spotLights;
    glm::vec2 uvPan = {0, 0};
    glm::vec2 uvScale2 = {1, 1};
    glm::vec2 normalUVScale2 = {1, 1};
    std::vector<ShaderParameter> shaderParameters;
    float uvScale = 1;
    bool depthTest = true;
    bool skinnedDraw = false;
    std::optional<glm::ivec2> viewPortDimensions;
    std::string subroutineFragBind = "";
    bool castShadow = true;
    uint32_t instanceCount = 0;

    // This is a customizable callback which will be called when the scene is rendered.
    // E.g. for setting specific shader variables.
    std::function<void(MeshDrawData mdd)> onRender;

};


#endif //MESHDRAWDATA_H
