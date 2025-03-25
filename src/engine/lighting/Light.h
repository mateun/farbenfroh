//
// Created by mgrus on 11.02.2025.
//

#ifndef LIGHT_H
#define LIGHT_H
#include <glm\glm.hpp>
#include <engine/graphics/FrameBuffer.h>
#include <vector>


class Shader;
class Camera;

enum class LightType {
  Directional,
  Point,
  Spot,
  };



struct PointLightData {
  float constant = 0.05;
  float linear = 0.09;
  float quadratic = 0.032;
};

class Light {
public:
  LightType type;
  glm::vec3 location;
  glm::vec3 lookAtTarget;
  glm::vec3 color = {1, 1,1};
  bool castsShadow = false;
  float shadowBias = 0.004;

  PointLightData pointLightData;

  // Stays zero if we do not cast shadows from this light
  std::unique_ptr<FrameBuffer> shadowMapFBO = nullptr;
  glm::vec3 _direction = {1, -1, -1};

  void quickDebugManipulation(float& left, float& right, float& bottom, float& top, float& x,
                             float& f) const;

  glm::mat4 getShadowProjectionMatrix(const Camera* fittingTarget = nullptr) const;
  glm::mat4 getShadowViewMatrix(const Camera* fittingTarget = nullptr) const;
  glm::mat4 getShadowViewProjectionMatrix(const Camera* fittingTarget = nullptr) const;

  // Shadowmap debugging related
  void initFrustumDebugging(Camera* fittingTarget);
  void updateFrustumDebugging(Camera *fittingTarget);
  void renderWorldFrustum(Camera* viewCamera, const Camera* fittingTarget);
  std::vector<glm::vec3> getLightFrustumFittedToCamera(const Camera* fittingTarget);

  // We need to bind the shadowmap of this light into
  // a texture unit slot, so while rendering
  // we can do the shadow lookup.
  void bindShadowMap(int i);

  void calculateDirectionFromCurrentLocationLookat();

private:


  // Holds vertex data for debug drawing etc.
  GLuint viewCameraFrustumVAO;
  GLuint lightFrustumVAO;
  GLuint targetPositionBuffer;
  GLuint positionBuffer;
  Shader * frustumShader = nullptr;
};


struct DirectionalLightData {
  glm::vec3 direction;
  glm::vec3 diffuseColor;
};




#endif //LIGHT_H
