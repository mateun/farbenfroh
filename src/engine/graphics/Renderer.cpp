//
// Created by mgrus on 25.03.2025.
//

#include "Renderer.h"
#include <ranges>
#include <GL\glew.h>
#include <engine/graphics/Camera.h>
#include <engine/graphics/Bitmap.h>
#include <engine/graphics/ErrorHandling.h>
#include <engine/lighting/Light.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/Shader.h>

#include "StatefulRenderer.h"


void Renderer::drawMesh(const MeshDrawData &drawData) {
    glBindVertexArray(drawData.mesh->vao);
    GL_ERROR_EXIT(123434);
    drawData.shader->bind();
    GL_ERROR_EXIT(1234341);

    if (drawData.texture) {
        drawData.texture->bindAt(0);
    } else {
        drawData.shader->setVec4Value(drawData.color, "singleColor");
    }

    if (drawData.normalMap) {
        // Normal map starts at 13...
        drawData.normalMap->bindAt(13);
        GL_ERROR_EXIT(981);
    } else {
        //getDefaultNormalMap()->bindAt(13);
    }

    // Lighting related:
    {
        // Directional
        int lightIndex = 0;
        for (auto directionalLight : drawData.directionalLights) {

            drawData.shader->setVec3Value(directionalLight->lookAtTarget - directionalLight->location, "directionalLightData[" + std::to_string(lightIndex) + "].direction");
            drawData.shader->setVec3Value(directionalLight->color, "directionalLightData[" + std::to_string(lightIndex) + "].diffuseColor");
            drawData.shader->setMat4Value(directionalLight->getShadowViewProjectionMatrix(drawData.camera), "directionalLightData[" + std::to_string(lightIndex) + "].mat_view_proj");
            directionalLight->bindShadowMap(lightIndex+1);
            drawData.shader->setFloatValue(directionalLight->shadowBias, "shadowBias");
            lightIndex++;
            GL_ERROR_EXIT(982);
        }
        drawData.shader->setIntValue(drawData.directionalLights.size(), "numDirectionalLights");

        // Point
        lightIndex = 0;
        for (auto l : drawData.pointLights) {
            drawData.shader->setVec3Value(l->location, "pointLightData[" + std::to_string(lightIndex) + "].position");
            drawData.shader->setFloatValue(l->pointLightData.constant, "pointLightData[" + std::to_string(lightIndex) + "].constant");
            drawData.shader->setFloatValue(l->pointLightData.linear, "pointLightData[" + std::to_string(lightIndex) + "].linear");
            drawData.shader->setFloatValue(l->pointLightData.quadratic, "pointLightData[" + std::to_string(lightIndex) + "].quadratic");
            drawData.shader->setVec3Value(l->color, "pointLightData[" + std::to_string(lightIndex) + "].diffuseColor");
//            drawData.shader->setMat4Value(l->getShadowViewProjectionMatrix(), "pointLightData[" + std::to_string(lightIndex) + "].mat_view_proj");
            l->bindShadowMap(lightIndex+3);
            lightIndex++;
            GL_ERROR_EXIT(982);
        }
        drawData.shader->setIntValue(drawData.pointLights.size(), "numPointLights");

        // TODO Spot


    }


    drawData.shader->setVec4Value(drawData.tint, "tint");
    drawData.shader->setVec2Value(drawData.uvPan, "uvPan");
    drawData.shader->setVec2Value(drawData.uvScale2, "uvScale2");
    drawData.shader->setVec2Value(drawData.normalUVScale2, "normalUVScale2");
    drawData.shader->setFloatValue(drawData.uvScale, "uvScale");

    GL_ERROR_EXIT(983);


    // Setting transformation matrices
    {
        using namespace glm;

        // Do we have a complete world transform matrix already set?
        // Then we just use it!
        // Otherwise we build it ourselves
        if (drawData.worldTransform.has_value()) {
            drawData.shader->setMat4Value(drawData.worldTransform.value(), "mat_world");
        } else {
            // Object to world transformation
            mat4 mattrans = translate(mat4(1), drawData.location);
            mat4 matscale = glm::scale(mat4(1),drawData.scale);
            mat4 matworld = glm::mat4(1);

            // For rotation we check if we have a rotation matrix set.
            if (drawData.rotationMatrix.has_value()) {
                matworld = mattrans * (drawData.rotationMatrix.value()) * matscale;

            } else {
                mat4 matrotX = rotate(mat4(1), radians(drawData.rotationEulers.x), {1, 0, 0} );
                mat4 matrotY = rotate(mat4(1), radians(drawData.rotationEulers.y), {0, 1, 0} );
                mat4 matrotZ = rotate(mat4(1), radians(drawData.rotationEulers.z), {0, 0, 1} );
                matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;

            }
            drawData.shader->setMat4Value(matworld, "mat_world");

        }
        drawData.shader->setMat4Value(drawData.camera_shared->getViewMatrix(), "mat_view");
        drawData.shader->setMat4Value(drawData.camera_shared->getProjectionMatrix(drawData.viewPortDimensions), "mat_projection");

        drawData.shader->setVec3Value(drawData.camera_shared->getUp(), "cameraUp");
        drawData.shader->setVec3Value(drawData.camera_shared->getRight(), "cameraRight");

    }
    GL_ERROR_EXIT(9930);

    if (drawData.skinnedDraw) {
        if (!drawData.boneMatrices.empty()) {
            drawData.shader->setMat4Array(drawData.boneMatrices, "u_BoneMatrices");
        }
    }

    if (!drawData.depthTest) {
        glDisable(GL_DEPTH_TEST);
    }

    if (!drawData.subroutineFragBind.empty()) {
        auto index= glGetSubroutineIndex(drawData.shader->handle, GL_FRAGMENT_SHADER, drawData.subroutineFragBind.c_str());
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);
    }


    // Apply all further parameters, specific to this mesh:
    for (auto shaderParam : drawData.shaderParameters) {
        std::visit([shaderParam, drawData](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, float>) {
                drawData.shader->setFloatValue(value, shaderParam.name);
            } else if constexpr (std::is_same_v<T, int>) {
                drawData.shader->setIntValue(value, shaderParam.name);
            } else if constexpr (std::is_same_v<T, glm::vec2>) {
                drawData.shader->setVec2Value(value, shaderParam.name);
            } else if constexpr (std::is_same_v<T, glm::vec3>) {
                drawData.shader->setVec3Value(value, shaderParam.name);
            } else if constexpr (std::is_same_v<T, glm::vec4>) {
                drawData.shader->setVec4Value(value, shaderParam.name);
            } else if constexpr (std::is_same_v<T, glm::mat4>) {
                drawData.shader->setMat4Value(value, shaderParam.name);
            }
        }, shaderParam.value);
    }

    // Check to see if we must set a specific viewport and/or scissor rect:
    if (drawData.setViewport) {
        glViewport(drawData.viewport.x, drawData.viewport.y, drawData.viewport.z, drawData.viewport.w);
    }

    if (drawData.instanceCount > 0) {
        //glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, drawData.instanceCount);
        glDrawElementsInstanced(GL_TRIANGLES, drawData.mesh->indices.size(), GL_UNSIGNED_INT, nullptr, drawData.instanceCount );
    } else {
        glDrawElements(GL_TRIANGLES, drawData.mesh->indices.size(), drawData.mesh->indexDataType, nullptr);
    }
    GL_ERROR_EXIT(9931);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    if (!drawData.depthTest) {
        glEnable(GL_DEPTH_TEST);
    }
    glBindVertexArray(0);

    if (drawData.setViewport) {
        glViewport(0, 0, getApplication()->scaled_width(), getApplication()->scaled_height());
    }

}



/**
 * This method collects and interprets the incoming widget drawing command
 * but does not immediately call GL to render the mesh.
 * It does collect the drawData in a list to formulate batched drawing call
 * efficiently later.
 *
 * @param mdd The draw data on which to base the draw call on.
 */
void Renderer::drawWidgetMeshDeferred(MeshDrawData mdd, const Widget * widget) {
    batchedDrawData_[widget].push_back(mdd);
}

void Renderer::submitDeferredWidgetCalls() {
    // TODO sort the drawData items
    // by useful criteria, e.g. the texture.
    // Then we can collect all mesh positions and uvs and
    // put them into one big vbo.
    // Attention: the drawData items (in terms of location) are
    // all in relation to the respective widgets child space.
    // So to actually calculate the correct world space location we would need
    // to walk the complete widget graph.

    for (auto& mdds: batchedDrawData_ | std::views::values) {
        // TODO temp: for now just call the imm. interface:
        for (auto& mdd: mdds) {
            drawMesh(mdd);
        }
    }
    batchedDrawData_.clear();
}

/**
* Generates a normal map just pointing outwards.
* Can be used if no real normal map is available and we
* avoid shader branching.
*/
std::shared_ptr<Texture> Renderer::getDefaultNormalMap() {
    static std::shared_ptr<Texture> normalMap = nullptr;
    if (!normalMap) {
        // 1-pixel default normal map (0,0,1)
        GLubyte defaultNormalPixel[] = {128, 128, 255}; // RGB = (0.5, 0.5, 1.0)

        GLuint defaultNormalMapTex;
        glGenTextures(1, &defaultNormalMapTex);
        glBindTexture(GL_TEXTURE_2D, defaultNormalMapTex);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, defaultNormalPixel);

        // Set parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        auto bm = new Bitmap();
        bm->height = 1;
        bm->width = 1;
        bm->pixels = defaultNormalPixel;
        return std::make_shared<Texture>(defaultNormalMapTex, bm);
    }

    return normalMap;

}

