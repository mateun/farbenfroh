//
// Created by mgrus on 24.03.2025.
//

#include "Camera.h"
#include <engine/algo/PositionProvider.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Application.h"

Camera::Camera(CameraType type): type_(type) {
}

/**
* Returns the 8 corners of this cameras view frustum in world coordinates.
*/
std::vector<glm::vec3> Camera::getFrustumWorldCorners() const {
    auto ltnw = frustumToWorld(     glm::vec4{-1,1,-1, 1});
    auto rtnw = frustumToWorld(    glm::vec4{1,1,-1, 1});
    auto lbnw = frustumToWorld(  glm::vec4{-1,-1,-1, 1});
    auto rbnw = frustumToWorld( glm::vec4{1,-1,-1, 1});
    auto ltfw = frustumToWorld(      glm::vec4{-1,1,1, 1});
    auto rtfw = frustumToWorld(     glm::vec4{1,1,1, 1});
    auto lbfw = frustumToWorld(       glm::vec4{-1,-1,1, 1});
    auto rbfw = frustumToWorld(  glm::vec4{1,-1,1, 1});

    return {ltnw, rtnw, lbnw, rbnw, ltfw, rtfw, lbfw, rbfw};
}

float Camera::getMaxFrustumDiagonal() {
    auto ltnw = frustumToWorld(     glm::vec4{-1,1,-1, 1});
    auto rtnw = frustumToWorld(    glm::vec4{1,1,-1, 1});
    auto lbnw = frustumToWorld(  glm::vec4{-1,-1,-1, 1});
    auto rbnw = frustumToWorld( glm::vec4{1,-1,-1, 1});
    auto ltfw = frustumToWorld(      glm::vec4{-1,1,1, 1});
    auto rtfw = frustumToWorld(     glm::vec4{1,1,1, 1});
    auto lbfw = frustumToWorld(       glm::vec4{-1,-1,1, 1});
    auto rbfw = frustumToWorld(  glm::vec4{1,-1,1, 1});

    return distance(ltnw, rbfw);

}

void Camera::addPostProcessEffect(PostProcessEffect *effect) {
    postProcessEffects.push_back(effect);
}

std::vector<PostProcessEffect *> Camera::getPostProcessEffects() const {
    return postProcessEffects;
}

void Camera::setProjectionMetrics(float left, float right, float bottom, float top) {
    left_ = left;
    right_ = right;
    bottom_ = bottom;
    top_ = top;
}


void Camera::setInitialForward(glm::vec3 fwd) {
    _initialForward = fwd;
}

void Camera::follow(const std::shared_ptr<PositionProvider>& followedObject, glm::vec3 offset) {
    this->followedObject = followedObject.get();
    this->followOffset = offset;
    this->followDirection = glm::normalize(followedObject->getPosition3D() - location);
}

void Camera::updateFollow() {
    if (followedObject) {
        updateLocation(followedObject->getPosition3D() + followOffset);
        this->followDirection = glm::normalize(followedObject->getPosition3D() - location);
        updateLookupTarget(followedObject->getPosition3D());
    }
}

void Camera::updateLocation(glm::vec3 loc) {
    location = loc;
}

void Camera::updateLookupTarget(glm::vec3 t) {
    lookAtTarget = t;
}

glm::mat4 Camera::getViewMatrix() const {
    return lookAt((location), (lookAtTarget), glm::vec3(0, 1,0));
}

glm::vec3 Camera::getInitialFoward() const {
    return _initialForward;
}

glm::vec3 Camera::getForward() const {
    return normalize(lookAtTarget - location);
}

glm::vec3 Camera::getRight() const {
    return normalize(cross( getForward(), {0, 1, 0}));
}

glm::vec3 Camera::getUp() const {
    return normalize(cross(getRight(), getForward() ));
}

/**
* viewCamera    Is the camera of which
*/
glm::vec4 Camera::frustumToWorld(glm::vec4 ndc) const {
    // TODO prepare for cascading shadow maps, we need to be flexible with the near far plane of our camera.
    // We construct our own perspective matrix here for a range of test near/far plane combinations
    auto flexiProj = glm::perspectiveFov<float>(glm::radians(50.0f), getApplication()->scaled_width(), getApplication()->scaled_height(),nearPlane,farPlane);
    auto temp = inverse( flexiProj* this->getViewMatrix()) * ndc;
    temp /= temp.w;
    return temp;
}

glm::mat4 Camera::getProjectionMatrix(std::optional<glm::ivec2> widthHeightOverride,
    std::optional<float> fovOverride) const {
    auto w = getApplication()->scaled_width();
    auto h = getApplication()->scaled_height();
    if (widthHeightOverride.has_value()) {
        w = widthHeightOverride.value().x;
        h = widthHeightOverride.value().y;
    }

    float fov = 30;
    if (fovOverride.has_value()) {
        fov = fovOverride.value();
    }

    if (type_ == CameraType::Ortho) {
        return glm::ortho<float>(0, w, 0, h, nearPlane, farPlane);
    }

    if (type_ == CameraType::OrthoGameplay) {
        return glm::ortho<float>(-10, 10, -8, 8, nearPlane, farPlane);
    }

    if (type_ == CameraType::Perspective) {
        return glm::perspectiveFov<float>(glm::radians(fov), w, h, nearPlane, farPlane);
    }
    return glm::mat4(1.0f);
}

void Camera::updateNearFar(float nearPlane, float farPlane) {
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
}
