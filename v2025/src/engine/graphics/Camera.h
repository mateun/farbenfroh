//
// Created by mgrus on 24.03.2025.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <GL\glew.h>
#include <vector>
#include <glm\glm.hpp>
#include <memory>
#include <optional>
#include <engine/graphics/Shader.h>

class PostProcessEffect;
class PositionProvider;


enum class CameraType {
    Ortho,
    Perspective,
    OrthoGameplay,
};




class Camera {

public:

    Camera(CameraType type = CameraType::Perspective);
    //~Camera();

    std::vector<glm::vec3> getFrustumWorldCorners() const;

    float getMaxFrustumDiagonal();
    void addPostProcessEffect(PostProcessEffect* effect);
    std::vector<PostProcessEffect*> getPostProcessEffects() const;
    void setProjectionMetrics(float left, float right, float bottom, float top);
    void setInitialForward(glm::vec3 fwd);
    void follow(const std::shared_ptr<PositionProvider>& followedObject, glm::vec3 offset);
    void updateFollow();
    void updateLocation(glm::vec3 loc);
    void updateLookupTarget(glm::vec3 t);
    glm::mat4 getViewMatrix() const;
    glm::vec3 getInitialFoward() const;
    glm::vec3 getForward() const;
    glm::vec3 getUp() const;
    glm::vec3 getRight() const;
    glm::vec4 frustumToWorld(glm::vec4 ndc) const;
    glm::mat4 getProjectionMatrix(std::optional<glm::ivec2> widthHeightOverride = std::nullopt, std::optional<float> fovOverride = 50.0f) const;
    void updateNearFar(float nearPlane, float farPlane);

    glm::vec3 location;
    glm::vec3 lookAtTarget;
    glm::vec3 _initialForward;
    PositionProvider* followedObject = nullptr;
    glm::vec3 followOffset;
    glm::vec3 followDirection;
    float nearPlane = 0.1f;
    float farPlane = 500.0f;
    GLuint positionBuffer = 0;
    GLuint shadowPositionBuffer = 0;
    Shader* frustumShader = nullptr;
    CameraType type_;
    float left_ = -1;
    float right_= -1;
    float bottom_ = -1;
    float top_ = -1;


private:
    // Can be used to debug draw the cameras frustum in world space,
    // e.g. for shadow mapping debugging.
    GLuint worldFrustumVAO = 0;

    // Holds the positions of the shadow camera frustum, mainly for debug
    // drawing.
    GLuint shadowCamFrustumVAO = 0;

    std::vector<PostProcessEffect*> postProcessEffects;
};



#endif //CAMERA_H
