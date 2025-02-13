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

class Light {
public:
  LightType type;
  glm::vec3 location;
  glm::vec3 lookAtTarget;
  glm::vec3 color;
  bool castsShadow = false;

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

struct PointLightData {
  glm::vec3 position;
  float constant;
  float linear;
  float quadratic;

};



#endif //LIGHT_H
