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
  bool castsShadow = false;

  // Stays zero if we do not cast shadows from this light
  FrameBuffer* shadowMapFBO = nullptr;


};



#endif //LIGHT_H
