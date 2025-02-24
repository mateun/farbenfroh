//
// Created by mgrus on 11.02.2025.
//

#ifndef LIGHT_H
#define LIGHT_H
#include <glm\glm.hpp>


enum class LightType {
  Directional,
  Point,
  Spot,
  };

struct FrameBuffer;

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

  PointLightData pointLightData;

  // Stays zero if we do not cast shadows from this light
  FrameBuffer* shadowMapFBO = nullptr;

  glm::mat4 getViewProjectionMatrix() const;

  // We need to bind the shadowmap of this light into
  // a texture unit slot, so while rendering
  // we can do the shadow lookup.
  void bindShadowMap(int i);
};


struct DirectionalLightData {
  glm::vec3 direction;
  glm::vec3 diffuseColor;
};




#endif //LIGHT_H
