//
// Created by mgrus on 11.02.2025.
//

#include "Light.h"
#include "graphics.h"

glm::mat4 Light::getViewProjectionMatrix() const {

  	Camera directionalLightCam;
    directionalLightCam.location = location;
    directionalLightCam.lookAtTarget = lookAtTarget;

	// Default create an ortho projection for a directional light.
	glm::mat4 projectionMatrix = glm::ortho<float>(-28, 28, -28, 28, 1.0f, 43.0f);

	// Make it a perspective matrix in case we are a point light.
	if (type == LightType::Point) {
  		projectionMatrix = glm::perspectiveFov<float>(glm::radians(50.0f), shadowMapFBO->texture->bitmap->width, shadowMapFBO->texture->bitmap->height, 1.0f, 10);
  	}

	return projectionMatrix * directionalLightCam.getViewMatrix();
 }

void Light::bindShadowMap(int unitIndex) {
	if (shadowMapFBO && shadowMapFBO->handle) {
		glActiveTexture(GL_TEXTURE0 + unitIndex);
		glBindTexture(GL_TEXTURE_2D, shadowMapFBO->texture->handle);
	}
}
