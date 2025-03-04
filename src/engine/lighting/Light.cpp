//
// Created by mgrus on 11.02.2025.
//

#include "Light.h"
#include "graphics.h"


glm::vec3 Light::findCenterForWorldPositions(std::vector<glm::vec3> worldCorners) const {
	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& v : worldCorners)
	{
		center += glm::vec3(v);
	}
	center /= worldCorners.size();
	return center;
}

glm::mat4 Light::getProjectionMatrix(Camera* fittingTarget) const {

	if (!fittingTarget) {
		return glm::ortho<float>(-28, 28, -28, 28, 1.0f, 43.0f);
	}

	// We fit the "camera" frustum of our light into the frustum of the view camera (fitting target),
	// so we make best use of the resolution and we do not calculate shadows where we are not looking currently anyway.
	auto worldCorners = fittingTarget->getFrustumWorldCorners();

	// To do this we need to find a projection of the camera space into our own light view space.
	// We do this by construction a lookat matric, which needs a direction (our light direction, easy)
	// and a point in space where we are located.
	// As we are meant to be travelling with the fitting target, we do not use
	// our own "location" property here, but we need to derive it based on the position of
	// the fitting target.
	auto lightViewMatrix = getViewMatrix(fittingTarget);

	// From world into shadow camera view coordinates.
	std::vector<glm::vec3> lightViewCorners;
	for (auto worldCorner : worldCorners) {
		auto lv = lightViewMatrix * glm::vec4(worldCorner, 1);
		lightViewCorners.push_back(lv);
	}

	// Finally we take the min x, y, z to have the correct dimensions of our frustum.
	float left = findMin(lightViewCorners, "x");
	float right = findMax(lightViewCorners, "x");
	float bottom = findMin(lightViewCorners, "y");
	float top = findMax(lightViewCorners, "y");
	float n = findMin(lightViewCorners, "z");
	float f = findMax(lightViewCorners, "z");

	// Pushing/Pulling the near/far planes to accomodate for better scene coverage.
	constexpr float zMult = 1.0f;

	if (n < 0)
	{
		n *= zMult;
	}
	else
	{
		n /= zMult;
	}
	if (f < 0)
	{
		f /= zMult;
	}
	else
	{
		f *= zMult;
	}

	return glm::ortho<float>(left, right, bottom, top, n, f);

 }

glm::mat4 Light::getViewMatrix(Camera *fittingTarget) const {
	// TODO think of this later, what can we do?
	if (!fittingTarget) {
		throw new std::runtime_error("No fitting target provided");
	}

	auto worldCorners = fittingTarget->getFrustumWorldCorners();
	glm::vec3 center = findCenterForWorldPositions(worldCorners);
	glm::vec3 eye = center + normalize(_direction);
	return lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Light::getViewProjectionMatrix(Camera *fittingTarget) const {
	return getProjectionMatrix(fittingTarget) * getViewMatrix(fittingTarget);
}

/**
* Initializes the VAOs for both the view camera frustum (i.e. the fitting target)
* and our own frustum.
*/
void Light::initFrustumDebugging(Camera* fittingTarget) {
    if (viewCameraFrustumVAO == 0) {

        // Initialize the frustum VAO of our view camera:
        glGenVertexArrays(1, &viewCameraFrustumVAO);
        glBindVertexArray(viewCameraFrustumVAO);
    	GL_ERROR_EXIT(21111)

    	auto worldCorners = fittingTarget->getFrustumWorldCorners();

        std::vector<float> posFlat;        for (auto worldPos : worldCorners) {
            posFlat.push_back(worldPos.x);
            posFlat.push_back(worldPos.y);
            posFlat.push_back(worldPos.z);
        }

        std::vector<uint16_t> indicesFlat  {
            0, 1,
            0, 2,
            1, 3,
            2, 3,
            0, 4,
            1, 5,
            2, 6,
            3, 7,
            4, 5,
            4, 6,
            5, 7,
            6, 7

        };

        glGenBuffers(1, &targetPositionBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, targetPositionBuffer);
        glBufferData(GL_ARRAY_BUFFER, posFlat.size() * 4, posFlat.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
    	GL_ERROR_EXIT(21112)

        GLuint vboIndices;
        glGenBuffers(1, &vboIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesFlat.size() * 2, indicesFlat.data(), GL_DYNAMIC_DRAW);
    	GL_ERROR_EXIT(21113)

        glBindVertexArray(0);

        // Next we prepare the VAO for our shadow camera frustum:
        glGenVertexArrays(1, &lightFrustumVAO);
        glBindVertexArray(lightFrustumVAO);
    	GL_ERROR_EXIT(21114)

		auto lightViewMatrix = getViewMatrix(fittingTarget);
    	std::vector<glm::vec3> lightFrustumWorldCorners;
    	for (auto fittingTargetPos : worldCorners) {
    		auto lightFrustumPos = (lightViewMatrix) * glm::vec4(fittingTargetPos, 1);
    		lightFrustumWorldCorners.push_back(lightFrustumPos);
    	}

        // The we take the min x, y to have the correct dimensions.
        float left = findMin(lightFrustumWorldCorners, "x");
        float right = findMax(lightFrustumWorldCorners, "x");
        float bottom = findMin(lightFrustumWorldCorners, "y");
        float top = findMax(lightFrustumWorldCorners, "y");
        float n = findMin(lightFrustumWorldCorners, "z");
        float f = findMax(lightFrustumWorldCorners, "z");

    	// Now we have found min/max, lets transform these back into world pos:
		glm::vec3 leftTopNearWorld = inverse(lightViewMatrix) * glm::vec4(left, top, n, 1);
    	glm::vec3 rightTopNearWorld = inverse(lightViewMatrix) * glm::vec4(right, top, n, 1);
    	glm::vec3 leftBottomNearWorld = inverse(lightViewMatrix) * glm::vec4(left, bottom, n, 1);
    	glm::vec3 rightBottomNearWorld = inverse(lightViewMatrix) * glm::vec4(right, bottom, n, 1);
    	glm::vec3 leftTopFarWorld = inverse(lightViewMatrix) * glm::vec4(left, top, f, 1);
    	glm::vec3 rightTopFarWorld = inverse(lightViewMatrix) * glm::vec4(right, top, f, 1);
    	glm::vec3 leftBottomFarWorld = inverse(lightViewMatrix) * glm::vec4(left, bottom, f, 1);
    	glm::vec3 rightBottomFarWorld = inverse(lightViewMatrix) * glm::vec4(right, bottom, f, 1);

        std::vector<float> shadowMapPosFlat = {
            leftTopNearWorld.x, leftTopNearWorld.y, leftTopNearWorld.z,									//  0
            rightTopNearWorld.x, rightTopNearWorld.y, rightTopNearWorld.z,								//  1
            leftBottomNearWorld.x, leftBottomNearWorld.y, leftBottomNearWorld.z,						//  2
            rightBottomNearWorld.x, rightBottomNearWorld.y, rightBottomNearWorld.z,						//  3
            leftTopFarWorld.x, leftTopFarWorld.y, leftTopFarWorld.z,									//  4
            rightTopFarWorld.x, rightTopFarWorld.y, rightTopFarWorld.z,									//  5
            leftBottomFarWorld.x, leftBottomFarWorld.y, leftBottomFarWorld.z,							//  6
            rightBottomFarWorld.x, rightBottomFarWorld.y, rightBottomFarWorld.z,						//  7
        };

        std::vector<uint16_t> shadowIndicesFlat  {
            0, 1,
            0, 2,
            1, 3,
            2, 3,
            0, 4,
            1, 5,
            2, 6,
            3, 7,
            4, 5,
            4, 6,
            5, 7,
            6, 7

        };

        glGenBuffers(1, &positionBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
        glBufferData(GL_ARRAY_BUFFER, shadowMapPosFlat.size() * 4, shadowMapPosFlat.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        GL_ERROR_EXIT(877654)

        GLuint shadowVboIndices;
        glGenBuffers(1, &shadowVboIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shadowVboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, shadowIndicesFlat.size() * 2, shadowIndicesFlat.data(), GL_DYNAMIC_DRAW);
    	GL_ERROR_EXIT(21115)

        glBindVertexArray(0);


        // Initialize our frustum shader
        frustumShader = new Shader();
        frustumShader->initFromFiles("../assets/shaders/frustum_debug.vert", "../assets/shaders/frustum_debug.frag");
    }

}

void Light::renderWorldFrustum(Camera *viewCamera) {
	glBindVertexArray(viewCameraFrustumVAO);
	glUseProgram(frustumShader->handle);

	// Cyan for the camera frustum
	frustumShader->setVec4Value({1, 0, 1, 1}, "singleColor");
	frustumShader->setMat4Value(viewCamera->getViewMatrix(), "mat_view");
	frustumShader->setMat4Value(viewCamera->getProjectionMatrix(), "mat_projection");
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, 0);
	GL_ERROR_EXIT(12300)

	glBindVertexArray(lightFrustumVAO);
	// Red for the shadow map frustum
	frustumShader->setVec4Value({1, 0, 0, 1}, "singleColor");
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, 0);
	GL_ERROR_EXIT(12301)

	glBindVertexArray(0);

}

void Light::updateFrustumDebugging(Camera* fittingTarget) {
	auto worldCorners = fittingTarget->getFrustumWorldCorners();
	std::vector<float> posFlat;
	for (auto worldPos : worldCorners) {
		posFlat.push_back(worldPos.x);
		posFlat.push_back(worldPos.y);
		posFlat.push_back(worldPos.z);
	}

	glBindBuffer(GL_ARRAY_BUFFER, targetPositionBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, posFlat.size() * sizeof(float), posFlat.data());
	GL_ERROR_EXIT(1111)


	auto lightViewMatrix = getViewMatrix(fittingTarget);
	std::vector<glm::vec3> lightFrustumWorldCorners;
	for (auto fittingTargetPos : worldCorners) {
		auto lightFrustumPos = lightViewMatrix * glm::vec4(fittingTargetPos, 1);
		lightFrustumWorldCorners.push_back(lightFrustumPos);
	}

	  // The we take the min x, y to have the correct dimensions.
        float left = findMin(lightFrustumWorldCorners, "x");
        float right = findMax(lightFrustumWorldCorners, "x");
        float bottom = findMin(lightFrustumWorldCorners, "y");
        float top = findMax(lightFrustumWorldCorners, "y");
        float n = findMin(lightFrustumWorldCorners, "z");
        float f = findMax(lightFrustumWorldCorners, "z");

    	// Now we have found min/max, lets transform these back into world pos:
		glm::vec3 leftTopNearWorld = inverse(lightViewMatrix) * glm::vec4(left, top, n, 1);
    	glm::vec3 rightTopNearWorld = inverse(lightViewMatrix) * glm::vec4(right, top, n, 1);
    	glm::vec3 leftBottomNearWorld = inverse(lightViewMatrix) * glm::vec4(left, bottom, n, 1);
    	glm::vec3 rightBottomNearWorld = inverse(lightViewMatrix) * glm::vec4(right, bottom, n, 1);
    	glm::vec3 leftTopFarWorld = inverse(lightViewMatrix) * glm::vec4(left, top, f, 1);
    	glm::vec3 rightTopFarWorld = inverse(lightViewMatrix) * glm::vec4(right, top, f, 1);
    	glm::vec3 leftBottomFarWorld = inverse(lightViewMatrix) * glm::vec4(left, bottom, f, 1);
    	glm::vec3 rightBottomFarWorld = inverse(lightViewMatrix) * glm::vec4(right, bottom, f, 1);

        std::vector<float> shadowMapPosFlat = {
            leftTopNearWorld.x, leftTopNearWorld.y, leftTopNearWorld.z,									//  0
            rightTopNearWorld.x, rightTopNearWorld.y, rightTopNearWorld.z,								//  1
            leftBottomNearWorld.x, leftBottomNearWorld.y, leftBottomNearWorld.z,						//  2
            rightBottomNearWorld.x, rightBottomNearWorld.y, rightBottomNearWorld.z,						//  3
            leftTopFarWorld.x, leftTopFarWorld.y, leftTopFarWorld.z,									//  4
            rightTopFarWorld.x, rightTopFarWorld.y, rightTopFarWorld.z,									//  5
            leftBottomFarWorld.x, leftBottomFarWorld.y, leftBottomFarWorld.z,							//  6
            rightBottomFarWorld.x, rightBottomFarWorld.y, rightBottomFarWorld.z,						//  7
        };


	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, shadowMapPosFlat.size() * 4, shadowMapPosFlat.data());
	GL_ERROR_EXIT(1112)

}



float Light::findMin(const std::vector<glm::vec3> &positions, const std::string &coord) {
	float min = std::numeric_limits<float>::max();
	for (auto p : positions) {
		if (coord == "x") {
			if (p.x < min) {
				min = p.x;
			}
		} else if (coord == "y") {
			if (p.y < min) {
				min = p.y;
			}
		}
		else if (coord == "z") {
			if (p.z < min) {
				min = p.z;
			}
		}

	}

	return min;
}

float Light::findMax(const std::vector<glm::vec3> &positions, const std::string &coord) {
	float m = std::numeric_limits<float>::lowest();
	for (auto p : positions) {
		if (coord == "x") {
			if (p.x > m) {
				m = p.x;
			}
		} else if (coord == "y") {
			if (p.y > m) {
				m = p.y;
			}
		}
		else if (coord == "z") {
			if (p.z > m) {
				m = p.z;
			}
		}
	}

	return m;
}

void Light::bindShadowMap(int unitIndex) {
	if (shadowMapFBO && shadowMapFBO->handle) {
		glActiveTexture(GL_TEXTURE0 + unitIndex);
		glBindTexture(GL_TEXTURE_2D, shadowMapFBO->texture->handle);
	}
}

void Light::calculateDirectionFromCurrentLocationLookat() {
	_direction = glm::normalize(lookAtTarget - location);
}
