//
// Created by mgrus on 11.02.2025.
//

#include "Light.h"
#include "graphics.h"
#include "../../engine/algo/VectorUtils.h"


void Light::quickDebugManipulation(float &left, float &right, float &bottom, float &top, float &n, float &f) const {
//#define QUICK_DEBUG
#ifdef QUICK_DEBUG
	left = -10;
	right = 10;
	bottom = -10;
	top = 10;
	n = 0.1;
	f = 20;
#endif
}

glm::mat4 Light::getShadowProjectionMatrix(Camera* fittingTarget) const {

#ifdef QUICK_DEBUG
	return glm::ortho<float>(-10, 10, -10, 10, 0.1, 20);
#endif
	if (type == LightType::Spot) {
		throw std::runtime_error("Spotlight shadow mapping not implemented yet");
	}

	if (type == LightType::Point) {
		throw std::runtime_error("Pointlight shadow mapping not implemented yet");
	}

	if (type == LightType::Directional && !fittingTarget) {
		throw new std::runtime_error("Light is directional and we do not have provided a camera fitting target!");
	}

	// Experiment: only try this once
	static bool projectionMatrixPreCalculated = false;
	static glm::mat4 projMat = glm::mat4(1.0f);
	if (projectionMatrixPreCalculated) {
		return projMat;

	}

	projectionMatrixPreCalculated = true;

	// We fit the "camera" frustum of our light into the frustum of the view camera (fitting target),
	// so we make best use of the resolution and we do not calculate shadows where we are not looking currently anyway.
	auto worldCorners = fittingTarget->getFrustumWorldCorners();



	// To do this we need to find a projection of the camera space into our own light view space.
	// We do this by construction a lookat matric, which needs a direction (our light direction, easy)
	// and a point in space where we are located.
	// As we are meant to be travelling with the fitting target, we do not use
	// our own "location" property here, but we need to derive it based on the position of
	// the fitting target.
	auto lightViewMatrix = getShadowViewMatrix(fittingTarget);

	// From world into shadow camera view coordinates.
	std::vector<glm::vec3> lightViewCorners;
	for (auto worldCorner : worldCorners) {
		auto lv = lightViewMatrix * glm::vec4(worldCorner, 1);
		lightViewCorners.push_back(lv);
	}

	// Finally we take the min x, y, z to have the correct dimensions of our frustum.
	float left = VectorUtils::findMin(lightViewCorners, "x");
	float right = VectorUtils::findMax(lightViewCorners, "x");
	float bottom = VectorUtils::findMin(lightViewCorners, "y");
	float top = VectorUtils::findMax(lightViewCorners, "y");
	float n = VectorUtils::findMin(lightViewCorners, "z");
	float f = VectorUtils::findMax(lightViewCorners, "z");

	quickDebugManipulation(left, right, bottom, top , n, f);

	// float texelSize = (right - left) / 1024.0f;
	// float centerX = (left + right) / 2.0f;
	// float centerY = (bottom + top) / 2.0f;
	// centerX = round(centerX / texelSize) * texelSize;
	// centerY = round(centerY / texelSize) * texelSize;
	// float halfWidth = (right - left) * 0.5f;
	// float halfHeight = (top - bottom) * 0.5f;
	// left   = centerX - halfWidth;
	// right  = centerX + halfWidth;
	// bottom = centerY - halfHeight;
	// top    = centerY + halfHeight;

	// Pushing/Pulling the near/far planes to accommodate for better scene coverage.
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

	projMat = glm::ortho<float>(left, right, bottom, top, n, f);
	return projMat;

 }

glm::mat4 Light::getShadowViewMatrix(Camera *fittingTarget) const {

	// If we have no fitting target, we will just assume a generic position
	if (!fittingTarget) {
		glm::vec3 eye = {5, 10, 0};
		return lookAt(eye, eye + _direction, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	auto worldCorners = fittingTarget->getFrustumWorldCorners();
	glm::vec3 center = VectorUtils::findCenterForWorldPositions(worldCorners);


	// Quantized position
#ifdef QUANTIZED_SHADOW_VIEW
	{
		// Create a rotation-only matrix for the light's orientation
		glm::mat4 lightRotationMat = glm::lookAt(glm::vec3(0.0f), _direction, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat3 lightRotation = glm::mat3(lightRotationMat);

		// Transform center to light space
		glm::vec3 centerLS = lightRotation * center;

		float texelSize = abs((-20 - 20) / 256.0f);

		// Quantize the x and y components in light space (using gridSize, which might be the same as texelSize)
		float gridSize = texelSize;  // or another chosen value
		centerLS.x = round(centerLS.x / gridSize) * gridSize;
		centerLS.y = round(centerLS.y / gridSize) * gridSize;

		// Transform the quantized center back to world space
		glm::vec3 quantizedCenter = glm::transpose(lightRotation) * centerLS;

		// Compute the eye position and rebuild the view matrix
		glm::vec3 eye = quantizedCenter + glm::normalize(-_direction);
		return lookAt(eye, quantizedCenter, glm::vec3(0.0f, 1.0f, 0.0f));
	}
#else

	// Compute the eye position by offsetting along the inverse of the light's direction.
	glm::vec3 eye = center + glm::normalize(-_direction);
	return lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));

#endif


}

glm::mat4 Light::getShadowViewProjectionMatrix(Camera *fittingTarget) const {
	return getShadowProjectionMatrix(fittingTarget) * getShadowViewMatrix(fittingTarget);
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

    	auto lightFrustumCornersFitted = getLightFrustumFittedToCamera(fittingTarget);
    	std::vector<float> shadowMapPosFlat;
    	for (auto frustumPos : lightFrustumCornersFitted) {
    		shadowMapPosFlat.push_back(frustumPos.x);
    		shadowMapPosFlat.push_back(frustumPos.y);
    		shadowMapPosFlat.push_back(frustumPos.z);
    	}

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

/**
* Returns a vector of world positions of the view frustum of this light, tightly wrapped around
* the given fitting target camera.
*/
std::vector<glm::vec3> Light::getLightFrustumFittedToCamera(Camera* fittingTarget) {

	auto lightViewMatrix = getShadowViewMatrix(fittingTarget);
	std::vector<glm::vec3> lightFrustumWorldCorners;
	for (auto fittingTargetPos : fittingTarget->getFrustumWorldCorners()) {
		auto lightFrustumPos = (lightViewMatrix) * glm::vec4(fittingTargetPos, 1);
		lightFrustumWorldCorners.push_back(lightFrustumPos);
	}

	// The we take the min x, y to have the correct dimensions.
	float left = VectorUtils::findMin(lightFrustumWorldCorners, "x");
	float right = VectorUtils::findMax(lightFrustumWorldCorners, "x");
	float bottom = VectorUtils::findMin(lightFrustumWorldCorners, "y");
	float top = VectorUtils::findMax(lightFrustumWorldCorners, "y");
	float n = VectorUtils::findMin(lightFrustumWorldCorners, "z");
	float f = VectorUtils::findMax(lightFrustumWorldCorners, "z");

	quickDebugManipulation(left, right, bottom, top , n, f);

	glm::vec3 leftTopNearWorld = inverse(lightViewMatrix) * glm::vec4(left, top, n, 1);
	glm::vec3 rightTopNearWorld = inverse(lightViewMatrix) * glm::vec4(right, top, n, 1);
	glm::vec3 leftBottomNearWorld = inverse(lightViewMatrix) * glm::vec4(left, bottom, n, 1);
	glm::vec3 rightBottomNearWorld = inverse(lightViewMatrix) * glm::vec4(right, bottom, n, 1);
	glm::vec3 leftTopFarWorld = inverse(lightViewMatrix) * glm::vec4(left, top, f, 1);
	glm::vec3 rightTopFarWorld = inverse(lightViewMatrix) * glm::vec4(right, top, f, 1);
	glm::vec3 leftBottomFarWorld = inverse(lightViewMatrix) * glm::vec4(left, bottom, f, 1);
	glm::vec3 rightBottomFarWorld = inverse(lightViewMatrix) * glm::vec4(right, bottom, f, 1);
	return {leftTopNearWorld, rightTopNearWorld, leftBottomNearWorld, rightBottomNearWorld, leftTopFarWorld, rightTopFarWorld, leftBottomFarWorld, rightBottomFarWorld};

}

/**
* The view camera is the active camera through which we render this debug information.
* This is normally different from the camera which the scene was originally rendered from (represented by the
* viewCameraFrustumVAO). The name is misleading, as what was the viewCameraFrustum during the calculation of the
* view frustum, is now represented by the originalFittingTarget.
*/
void Light::renderWorldFrustum(Camera *viewCamera, Camera* originalFittingTarget) {
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

	// Now write some text at the corners of the frusta:
	static FBFont* font;
	if (!font) {
		font = new FBFont("../assets/font.bmp");
	}

	static Camera* uiCamera;
	if (!uiCamera) {
		uiCamera = new Camera();
		uiCamera->location = {0, 0, 1};
		uiCamera->lookAtTarget = {0, 0, -1};
		uiCamera->type = CameraType::Ortho;
	}

	lightingOff();
	bindCamera(uiCamera);

	flipUvs(false);
	{

		auto frustomWorldPositions = getLightFrustumFittedToCamera(originalFittingTarget);
		for (auto frustumPos : frustomWorldPositions) {
			glm::vec4 clipSpace = viewCamera->getProjectionMatrix() * viewCamera->getViewMatrix() * glm::vec4(frustumPos, 1);
			glm::vec4 ndc = clipSpace / clipSpace.w;
			glm::vec2 ss = {((ndc.x+1)/2) * scaled_width, ((ndc.y+1)/2)*scaled_height};
			char buf[175];
			sprintf_s(buf, 160, "%4.1f/%4.1f/%4.1f",
					  frustumPos.x, frustumPos.y, frustumPos.z);
			font->renderText(buf, {ss.x, ss.y, 0.9});
		}



	}


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

	auto lightFrustumCornersFitted = getLightFrustumFittedToCamera(fittingTarget);
	std::vector<float> shadowMapPosFlat;
	for (auto frustumPos : lightFrustumCornersFitted) {
		shadowMapPosFlat.push_back(frustumPos.x);
		shadowMapPosFlat.push_back(frustumPos.y);
		shadowMapPosFlat.push_back(frustumPos.z);
	}

	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, shadowMapPosFlat.size() * 4, shadowMapPosFlat.data());
	GL_ERROR_EXIT(1112)

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
