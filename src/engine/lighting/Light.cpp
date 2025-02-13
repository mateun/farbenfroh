//
// Created by mgrus on 11.02.2025.
//

#include "Light.h"
#include "graphics.h"

glm::mat4 Light::getViewProjectionMatrix() const {

  	Camera directionalLightCam;
    directionalLightCam.location = location;
    directionalLightCam.lookAtTarget = lookAtTarget;

	  if (type == LightType::Directional) {
    		directionalLightCam.type = CameraType::Ortho;
  	}

      return directionalLightCam.getProjectionMatrix() * directionalLightCam.getViewMatrix();
 }

void Light::bindShadowMap(int unitIndex) {
	if (shadowMapFBO && shadowMapFBO->handle) {
		glActiveTexture(GL_TEXTURE0 + unitIndex);
		glBindTexture(GL_TEXTURE_2D, shadowMapFBO->texture->handle);
	}
}
