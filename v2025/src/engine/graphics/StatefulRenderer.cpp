//
// Created by mgrus on 26.03.2025.
//

#include <GL\glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <engine/graphics/StatefulRenderer.h>
#include <engine/graphics/Camera.h>
#include <engine/lighting/Light.h>

#include "ErrorHandling.h"
#include "MeshDrawData.h"


void StatefulRenderer::drawGrid(GridData *gridData, glm::ivec2 viewPortDimensions, bool blurred) {

        glBindVertexArray(gridData->vao);
        glDefaultObjects->gridShader->bind();

        location(gridData->loc);

        using namespace glm;
        mat4 mattrans = translate(mat4(1), gridData->loc);
        mat4 matscale = glm::scale(mat4(1), vec3(gridData->scale));
        mat4 matrotX = glm::rotate(mat4(1), glm::radians(glDefaultObjects->currentRenderState->rot.x), {1, 0, 0} );
        mat4 matrotY = glm::rotate(mat4(1), glm::radians(glDefaultObjects->currentRenderState->rot.y), {0, 1, 0} );
        mat4 matrotZ = glm::rotate(mat4(1), glm::radians(glDefaultObjects->currentRenderState->rot.z), {0, 0, 1} );
        mat4 matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;


        glDefaultObjects->gridShader->setMat4Value(matworld, "mat_world");
        glDefaultObjects->gridShader->setMat4Value(gridData->camera->getViewMatrix(), "mat_view");
        glDefaultObjects->gridShader->setMat4Value(gridData->camera->getProjectionMatrix(viewPortDimensions), "mat_projection");
        glDefaultObjects->gridShader->setVec4Value(gridData->color, "singleColor");

        glDisable(GL_DEPTH_TEST);

        glDrawArrays(GL_LINES, 0, (gridData->numLines * 2) + (gridData->numLines * 2));
        GL_ERROR_EXIT(7654);
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);

}

DrawCall StatefulRenderer::createPlaneDrawCall() {
    DrawCall dc;
    dc.location = glDefaultObjects->currentRenderState->location;
    dc.rotation = glDefaultObjects->currentRenderState->rot;
    dc.scale = glDefaultObjects->currentRenderState->scale;
    dc.texture = glDefaultObjects->currentRenderState->texture;
    dc.foregroundColor = glDefaultObjects->currentRenderState->foregroundColor;
    dc.overrideAlpha = glDefaultObjects->currentRenderState->overrideAlpha;
    dc.flipUvs = glDefaultObjects->currentRenderState->flipUvs;
    dc.tilingOn = glDefaultObjects->currentRenderState->tilingOn;
    dc.shadows = glDefaultObjects->currentRenderState->shadows;
    dc.lightingOn = glDefaultObjects->currentRenderState->lightingOn;
    dc.uvScale = glDefaultObjects->currentRenderState->uvScale;
    dc.camera = glDefaultObjects->currentRenderState->camera;
    dc.shadowMapCamera = glDefaultObjects->currentRenderState->shadowMapCamera;
    return dc;
}

void StatefulRenderer::drawPlaneCallExecution(DrawCall dc) {
    // glBindVertexArray(glDefaultObjects->quadVAO);
    // if (dc.texture) {
    //     glDefaultObjects->texturedShaderUnlit->bind();
    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, dc.texture->handle());
    // } else {
    //     glDefaultObjects->singleColorShader->bind();
    //     glUniform4fv(1, 1, (float*) &dc.foregroundColor);
    // }
    //
    // if (dc.lightingOn) {
    //     glUniform1i(13, 1);
    // } else {
    //     glUniform1i(13, 0);
    // }
    //
    // glUniform1f(15, dc.overrideAlpha);
    //
    // // Manipulate the uvs for tiling.
    // if (dc.tilingOn) {
    //     glUniform1i(20, 0);
    //     float width_uv = (float)  glDefaultObjects->currentRenderState->tileData.tileWidth / (float) glDefaultObjects->currentRenderState->texture-width() ;
    //     float height_uv = (float) glDefaultObjects->currentRenderState->tileData.tileHeight / (float) glDefaultObjects->currentRenderState->texture->bitmap->height;
    //     int tileX = glDefaultObjects->currentRenderState->tileData.tileX;
    //     int tileY = glDefaultObjects->currentRenderState->tileData.tileY;
    //     float uvs[] = {
    //             tileX * width_uv , 1 - (tileY * height_uv),
    //             tileX * width_uv, 1- (tileY * height_uv) - height_uv,
    //             (tileX * width_uv) + width_uv, 1- (tileY * height_uv)- height_uv,
    //             (tileX * width_uv) + width_uv, 1 - (tileY * height_uv),
    //
    //     };
    //     glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    //     glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
    // }
    //
    // if (dc.shadows && dc.shadowPass) {
    //     // Render everything twice, first into the shadowmap buffer here
    //     glViewport(0, 0, 1024, 1024);
    //     prepareShadowMapTransformationMatrices(dc);
    //     glBindFramebuffer(GL_FRAMEBUFFER, glDefaultObjects->shadowMapFramebuffer);
    //     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    //     glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //     glViewport(0, 0, getApplication()->scaled_width(), getApplication()->scaled_height());
    // }
    //
    // if (!dc.shadowPass) {
    //     prepareTransformationMatrices(dc);
    //     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    // }
    //
    //
    //
    // if (dc.tilingOn) {
    //     glUniform1i(20, 0);
    //     float uvs[] = {
    //             0, 1,
    //             0, 0,
    //             1, 0,
    //             1, 1,
    //
    //     };
    //     glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    //     glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
    // }
    //
    // glUniform1f(15, 1);
    //
    // glUniform1f(21, dc.uvScale);
    //
    // glBindTexture(GL_TEXTURE_2D, 0);
    // glBindVertexArray(0);
}

std::unique_ptr<Texture> StatefulRenderer::createTextureFromFile(const std::string &fileName) {
    throw std::runtime_error("Not implemented");
}



void StatefulRenderer::scale(glm::vec3 val) {
}

void StatefulRenderer::textScale(glm::vec2 val) {
}

void StatefulRenderer::panUVS(glm::vec2 pan) {
}

void StatefulRenderer::location(glm::vec3 loc) {
}

void StatefulRenderer::bindTexture(Texture *tex) {
}

void StatefulRenderer::bindNormalMap(Texture *tex, int unit) {
}

void StatefulRenderer::bindSkyboxTexture(Texture *tex) {
}

void StatefulRenderer::bindMesh(Mesh *mesh) {
    throw std::runtime_error("Not implemented");
}

void StatefulRenderer::bindCamera(Camera *camera) {
}

void StatefulRenderer::bindShadowMapCamera(Camera *camera) {
}

void StatefulRenderer::font(gru::Bitmap *fontBitmap) {
}


GLuint StatefulRenderer::createQuadVAO(PlanePivot pivot) {
    throw std::runtime_error("Not implemented");
}

Result StatefulRenderer::createShader(const std::string &vsrc, const std::string &fsrc, Shader *target) {
    throw std::runtime_error("Not implemented");
    return {};
}

void StatefulRenderer::prepareTransformationMatrices(glm::mat4 matworld, Camera* camera, Camera* shadowMapCamera, bool shadows) {
    // using namespace glm;
    //
    // auto worldLoc = glGetUniformLocation(glDefaultObjects->currentRenderState->shader->handle, "mat_model");
    // auto viewLoc = glGetUniformLocation(glDefaultObjects->currentRenderState->shader->handle, "mat_view");
    // auto projLoc = glGetUniformLocation(glDefaultObjects->currentRenderState->shader->handle, "mat_projection");
    //
    // glUniformMatrix4fv(	worldLoc,1,GL_FALSE,value_ptr(matworld));
    // glUniformMatrix4fv( viewLoc, 1, GL_FALSE, value_ptr(viewMatrixForCamera(camera)));
    // glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projectionMatrixForCamera(camera)));
    //
    // if (glDefaultObjects->currentRenderState->skinnedDraw) {
    //     // Send bone matrices to the shader
    //
    //     // Get the location of the bone matrix uniform in the shader
    //     //GLint boneMatrixLocation = glGetUniformLocation(shaderProgram, "u_BoneMatrices");
    //     GLint boneMatrixLocation = 24;
    //
    //     // Ensure the boneMatrices vector has the correct size (MAX_BONES or fewer)
    //     //assert(boneMatrices.size() <= MAX_BONES);
    //
    //     glUniformMatrix4fv(boneMatrixLocation, glDefaultObjects->boneMatrices.size(), GL_FALSE, glm::value_ptr(glDefaultObjects->boneMatrices[0]));
    //
    //     //glUniformMatrix4fv(boneMatrixUniformLocation, MAX_BONES, GL_FALSE, &glDefaultObjects->boneMatrices[0][0]);
    //
    // }
    //
    // if (shadows) {
    //     auto dirLightLoc = glGetUniformLocation(glDefaultObjects->currentRenderState->shader->handle, "mat_vp_directional_light");
    //     auto directionalLightViewProjection = projectionMatrixForShadowmap(shadowMapCamera) * viewMatrixForCamera(shadowMapCamera);
    //     glUniformMatrix4fv( dirLightLoc, 1, GL_FALSE, value_ptr(directionalLightViewProjection));
    //     glActiveTexture(GL_TEXTURE1);
    //     glBindTexture(GL_TEXTURE_2D, glDefaultObjects->shadowMap->handle);
    // }
}


void StatefulRenderer::prepareTransformationMatrices(glm::vec3 location, glm::vec3 scale, glm::vec3 rotation, Camera* camera, Camera* shadowMapCamera, bool shadows) {
    // using namespace glm;
    //
    // // Object to world transformation
    // mat4 mattrans = translate(mat4(1), location);
    // mat4 matscale = glm::scale(mat4(1),scale);
    // mat4 matworld = glm::mat4(1);
    //
    // // For rotation we check if we have a rotation matrix set.
    // if (glDefaultObjects->currentRenderState->rotMatrix) {
    //     matworld = mattrans * (*glDefaultObjects->currentRenderState->rotMatrix) * matscale;
    //
    // } else {
    //     mat4 matrotX = glm::rotate(mat4(1), glm::radians(rotation.x), {1, 0, 0} );
    //     mat4 matrotY = glm::rotate(mat4(1), glm::radians(rotation.y), {0, 1, 0} );
    //     mat4 matrotZ = glm::rotate(mat4(1), glm::radians(rotation.z), {0, 0, 1} );
    //     matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;
    // }
    //
    // auto worldLoc = glGetUniformLocation(glDefaultObjects->currentRenderState->shader->handle, "mat_model");
    // auto viewLoc = glGetUniformLocation(glDefaultObjects->currentRenderState->shader->handle, "mat_view");
    // auto projLoc = glGetUniformLocation(glDefaultObjects->currentRenderState->shader->handle, "mat_projection");
    //
    // glUniformMatrix4fv(	worldLoc,1,GL_FALSE,value_ptr(matworld));
    //
    // glUniformMatrix4fv( viewLoc, 1, GL_FALSE, value_ptr(viewMatrixForCamera(camera)));
    // glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projectionMatrixForCamera(camera)));
    //
    // if (glDefaultObjects->currentRenderState->skinnedDraw) {
    //     // Send bone matrices to the shader
    //
    //     // Get the location of the bone matrix uniform in the shader
    //     GLint boneMatrixLocation = glGetUniformLocation(glDefaultObjects->currentRenderState->shader->handle, "u_BoneMatrices");
    //
    //     if (boneMatrixLocation  == -1) {
    //         exit(100);
    //     }
    //
    //
    //     // Sometimes we render before the initial update and so we do not have the boneMatrices filled yet:
    //     if (!glDefaultObjects->boneMatrices.empty()) {
    //         glUniformMatrix4fv(boneMatrixLocation, glDefaultObjects->boneMatrices.size(), GL_FALSE, glm::value_ptr(glDefaultObjects->boneMatrices[0]));
    //     }
    //
    //
    //
    // }

}

/**
 * All current information is retrieved from a pre-recorded
 * draw call.
 * @param dc The drawcall on which basis the transformation matrices are
 *           to be based upon.
 */
void StatefulRenderer::prepareTransformationMatrices(DrawCall dc) {
    // prepareTransformationMatrices(dc.location,
    //                               dc.scale,
    //                               dc.rotation,
    //                               dc.camera,
    //                              dc.shadowMapCamera,
    //                               dc.shadows);

}





void StatefulRenderer::prepareShadowMapTransformationMatrices(glm::vec3 location, glm::vec3 scale, glm::vec3 rotation, Camera* shadowMapCamera) {
    // using namespace glm;
    //
    // // Object to world transformation
    // mat4 mattrans = translate(mat4(1), location);
    // mat4 matscale = glm::scale(mat4(1), scale);
    // mat4 matrotX = glm::rotate(mat4(1), glm::radians(rotation.x), {1, 0, 0} );
    // mat4 matrotY = glm::rotate(mat4(1), glm::radians(rotation.y), {0, 1, 0} );
    // mat4 matrotZ = glm::rotate(mat4(1), glm::radians(rotation.z), {0, 0, 1} );
    // mat4 matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;
    //
    // glUniformMatrix4fv(	6,1,GL_FALSE,value_ptr(matworld));
    // GL_ERROR_EXIT(991);
    //
    // glUniformMatrix4fv( 7, 1, GL_FALSE, value_ptr(shadowMapCamera->getViewMatrix()));
    // GL_ERROR_EXIT(992);
    // glUniformMatrix4fv(8, 1, GL_FALSE, value_ptr(shadowMapCamera->getProjectionMatrix()));
    // GL_ERROR_EXIT(993);

}

void StatefulRenderer::prepareShadowMapTransformationMatrices(DrawCall& dc) {
    prepareShadowMapTransformationMatrices(dc.location, dc.scale, dc.rotation, dc.shadowMapCamera);

}

std::unique_ptr<FrameBuffer> StatefulRenderer::createFrameBuffer(int width, int height, bool hdr,
    bool additionalColorBuffer) {
    return nullptr;
}

std::unique_ptr<FrameBuffer> StatefulRenderer::createFrameBufferWithTexture(int width, int height,
    std::shared_ptr<Texture> colorTexture, std::shared_ptr<Texture> colorTexture2) {
    return nullptr;
}

void StatefulRenderer::activateFrameBuffer(const FrameBuffer *fb) {
}

GridData * StatefulRenderer::createGrid(int lines) {
    return nullptr;
}

void StatefulRenderer::prepareShadowMapTransformationMatrices() {
    prepareShadowMapTransformationMatrices(glDefaultObjects->currentRenderState->location,
                                           glDefaultObjects->currentRenderState->scale,
                                           glDefaultObjects->currentRenderState->rot,
                                           glDefaultObjects->currentRenderState->shadowMapCamera
                                           );

}

void StatefulRenderer::drawPlaneUnlit() {

    // if (glDefaultObjects->currentRenderState->textDraw) {
    //     glBindVertexArray(glDefaultObjects->textQuadVAO);
    //     GL_ERROR_EXIT(110)
    // } else {
    //     glBindVertexArray(glDefaultObjects->quadVAO);
    // }
    //
    // // Shader selection:
    // // We check if a specific shader has been set (forced), then we use it.
    // // Otherwise there is some logic which shader to choose as a best guess.
    // if (glDefaultObjects->currentRenderState->forcedShader) {
    //     (glDefaultObjects->currentRenderState->forcedShader)->bind();
    //     GL_ERROR_EXIT(44)
    //     // Assuming the forced shader uses a texture?!
    //     if (glDefaultObjects->currentRenderState->texture) {
    //         glActiveTexture(GL_TEXTURE0);
    //         glBindTexture(GL_TEXTURE_2D, glDefaultObjects->currentRenderState->texture->handle());
    //     }
    // } else {
    //     if (glDefaultObjects->currentRenderState->texture) {
    //         bindShader(glDefaultObjects->texturedShaderUnlit);
    //         GL_ERROR_EXIT(45)
    //         glActiveTexture(GL_TEXTURE0);
    //         GL_ERROR_EXIT(46)
    //         glBindTexture(GL_TEXTURE_2D, glDefaultObjects->currentRenderState->texture->handle());
    //         GL_ERROR_EXIT(47)
    //     } else {
    //         bindShader(glDefaultObjects->singleColorShader);
    //         glUniform4fv(1, 1, (float*) &glDefaultObjects->currentRenderState->foregroundColor);
    //         GL_ERROR_EXIT(123)
    //     }
    // }
    //
    // // Tinting
    // glUniform4fv(16, 1, (float*) &glDefaultObjects->currentRenderState->tint);
    // GL_ERROR_EXIT(124)
    //
    // // Flip UVs
    // glUniform1i(20, glDefaultObjects->currentRenderState->flipUvs ? 1 : 0);
    // GL_ERROR_EXIT(125)
    //
    // // UV Panning
    // glUniform1f(22, glDefaultObjects->currentRenderState->panUVS.x);
    // glUniform1f(23, glDefaultObjects->currentRenderState->panUVS.y);
    // GL_ERROR_EXIT(126)
    //
    // // Manipulate the uvs for tiling.
    // if (glDefaultObjects->currentRenderState->tilingOn) {
    //     //glUniform1i(20, 0);
    //     float width_uv = (float)  glDefaultObjects->currentRenderState->tileData.tileWidth / (float) glDefaultObjects->currentRenderState->texture->bitmap->width ;
    //     float height_uv = (float) glDefaultObjects->currentRenderState->tileData.tileHeight / (float) glDefaultObjects->currentRenderState->texture->bitmap->height;
    //     int tileX = glDefaultObjects->currentRenderState->tileData.tileX;
    //     int tileY = glDefaultObjects->currentRenderState->tileData.tileY;
    //     float tileOffsetX = (float) glDefaultObjects->currentRenderState->tileData.tileOffsetX / (float) glDefaultObjects->currentRenderState->texture->bitmap->width;
    //     float tileOffsetY = (float) glDefaultObjects->currentRenderState->tileData.tileOffsetY / (float) glDefaultObjects->currentRenderState->texture->bitmap->height;
    //     float uvs[] = {
    //             tileOffsetX + (tileX * width_uv) , ( 1 - tileOffsetY - (tileY * height_uv)),
    //             tileOffsetX + (tileX * width_uv), (1- tileOffsetY - (tileY * height_uv) - height_uv),
    //             tileOffsetX + (tileX * width_uv) + width_uv,  (1- tileOffsetY -  (tileY * height_uv)- height_uv),
    //             tileOffsetX + (tileX * width_uv) + width_uv, (1 - tileOffsetY - (tileY * height_uv)),
    //
    //     };
    //     glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    //     glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
    // }
    //
    // prepareTransformationMatrices();
    //
    // glUniform1f(21, glDefaultObjects->currentRenderState->uvScale);
    // GL_ERROR_EXIT(343)
    //
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    // GL_ERROR_EXIT(344)
    //
    //
    // if (glDefaultObjects->currentRenderState->tilingOn) {
    //     glUniform1i(20, 0);
    //     float uvs[] = {
    //             0, 1,
    //             0, 0,
    //             1, 0,
    //             1, 1,
    //
    //     };
    //     glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    //     glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
    //     GL_ERROR_EXIT(345)
    // }
    //
    // glBindTexture(GL_TEXTURE_2D, 0);
    // glBindVertexArray(0);

}


[[deprecated]]
void StatefulRenderer::drawPlaneLit(Light *directionalLight, const std::vector<Light *> &pointLights) {

    // if (glDefaultObjects->currentRenderState->textDraw) {
    //     glBindVertexArray(glDefaultObjects->textQuadVAO);
    //     GL_ERROR_EXIT(110)
    // } else {
    //     glBindVertexArray(glDefaultObjects->quadVAO);
    // }
    //
    // // Shader selection:
    // // We check if a specific shader has been set (forced), then we use it.
    // // Otherwise there is some logic which shader to choose as a best guess.
    // if (glDefaultObjects->currentRenderState->forcedShader) {
    //     glDefaultObjects->currentRenderState->forcedShader->bind();
    //     GL_ERROR_EXIT(44)
    //     // Assuming the forced shader uses a texture?!
    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, glDefaultObjects->currentRenderState->texture->handle());
    // } else {
    //     if (glDefaultObjects->currentRenderState->texture) {
    //         glDefaultObjects->texturedShaderLit->bind();
    //         GL_ERROR_EXIT(45)
    //         glActiveTexture(GL_TEXTURE0);
    //         GL_ERROR_EXIT(46)
    //         glBindTexture(GL_TEXTURE_2D, glDefaultObjects->currentRenderState->texture->handle());
    //         GL_ERROR_EXIT(47)
    //     } else {
    //         glDefaultObjects->singleColorShader->bind();
    //         glUniform4fv(1, 1, (float*) &glDefaultObjects->currentRenderState->foregroundColor);
    //     }
    // }
    //
    // if (glDefaultObjects->currentRenderState->lightingOn) {
    //     glUniform1i(13, 1);
    //     glUniform3fv(10, 1, (float *) &directionalLight->_direction);
    // } else {
    //     glUniform1i(13, 0);
    // }
    // GL_ERROR_EXIT(123)
    //
    // // Tinting
    // glUniform4fv(16, 1, (float*) &glDefaultObjects->currentRenderState->tint);
    // GL_ERROR_EXIT(124)
    //
    // // Flip UVs
    // glUniform1i(20, glDefaultObjects->currentRenderState->flipUvs ? 1 : 0);
    // GL_ERROR_EXIT(125)
    //
    // // UV Panning
    // glUniform1f(22, glDefaultObjects->currentRenderState->panUVS.x);
    // glUniform1f(23, glDefaultObjects->currentRenderState->panUVS.y);
    // GL_ERROR_EXIT(126)
    //
    // // Manipulate the uvs for tiling.
    // if (glDefaultObjects->currentRenderState->tilingOn) {
    //     //glUniform1i(20, 0);
    //     float width_uv = (float)  glDefaultObjects->currentRenderState->tileData.tileWidth / (float) glDefaultObjects->currentRenderState->texture->bitmap->width ;
    //     float height_uv = (float) glDefaultObjects->currentRenderState->tileData.tileHeight / (float) glDefaultObjects->currentRenderState->texture->bitmap->height;
    //     int tileX = glDefaultObjects->currentRenderState->tileData.tileX;
    //     int tileY = glDefaultObjects->currentRenderState->tileData.tileY;
    //     float tileOffsetX = (float) glDefaultObjects->currentRenderState->tileData.tileOffsetX / (float) glDefaultObjects->currentRenderState->texture->bitmap->width;
    //     float tileOffsetY = (float) glDefaultObjects->currentRenderState->tileData.tileOffsetY / (float) glDefaultObjects->currentRenderState->texture->bitmap->height;
    //     float uvs[] = {
    //             tileOffsetX + (tileX * width_uv) , ( 1 - tileOffsetY - (tileY * height_uv)),
    //             tileOffsetX + (tileX * width_uv), (1- tileOffsetY - (tileY * height_uv) - height_uv),
    //             tileOffsetX + (tileX * width_uv) + width_uv,  (1- tileOffsetY -  (tileY * height_uv)- height_uv),
    //             tileOffsetX + (tileX * width_uv) + width_uv, (1 - tileOffsetY - (tileY * height_uv)),
    //
    //     };
    //     glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    //     glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
    // }
    //
    // if (glDefaultObjects->currentRenderState->shadows) {
    //     // Render everything twice, first into the shadowmap buffer here
    //     glViewport(0, 0, 1024, 1024);
    //     prepareShadowMapTransformationMatrices();
    //     glBindFramebuffer(GL_FRAMEBUFFER, glDefaultObjects->shadowMapFramebuffer);
    //     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    //     glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //     glViewport(0, 0, window_width, window_height);
    // }
    //
    // prepareTransformationMatrices();
    //
    // glUniform1f(21, glDefaultObjects->currentRenderState->uvScale);
    //
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    // GL_ERROR_EXIT(344)
    //
    //
    // if (glDefaultObjects->currentRenderState->tilingOn) {
    //     glUniform1i(20, 0);
    //     float uvs[] = {
    //             0, 1,
    //             0, 0,
    //             1, 0,
    //             1, 1,
    //
    //     };
    //     glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    //     glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
    // }
    //
    //
    //
    // glBindTexture(GL_TEXTURE_2D, 0);
    // glBindVertexArray(0);

}

void StatefulRenderer::drawPlane(Light *directionalLight, const std::vector<Light *>& pointLights) {
    // if (!directionalLight && pointLights.empty()) {
    //     drawPlaneUnlit();
    //     return;
    // }
    //
    // drawPlaneLit(directionalLight, pointLights);
}

void StatefulRenderer::drawMesh() {
    throw std::runtime_error("Not implemented");
}

void StatefulRenderer::drawMeshIntoShadowMap(MeshDrawData dd, Light *l) {
    throw std::runtime_error("Not implemented");
}


[[deprecated("Use drawPlane(dirLight, pointLights) instead.")]]
void StatefulRenderer::drawPlane() {
    // if (glDefaultObjects->currentRenderState->deferred) {
    //     // Only create a draw call here.
    //     auto dc = createPlaneDrawCall();
    //     glDefaultObjects->currentRenderState->drawCalls.push_back(dc);
    //     return;
    // }
    //
    // if (glDefaultObjects->currentRenderState->textDraw) {
    //     glBindVertexArray(glDefaultObjects->textQuadVAO);
    //     GL_ERROR_EXIT(110)
    // } else {
    //     glBindVertexArray(glDefaultObjects->quadVAO);
    // }
    //
    // // Shader selection:
    // // We check if a specific shader has been set (forced), then we use it.
    // // Otherwise there is some logic which shader to choose as a best guess.
    // if (glDefaultObjects->currentRenderState->forcedShader) {
    //     bindShader(glDefaultObjects->currentRenderState->forcedShader);
    //     GL_ERROR_EXIT(44)
    //     if (glDefaultObjects->currentRenderState->texture) {
    //         glActiveTexture(GL_TEXTURE0);
    //         glBindTexture(GL_TEXTURE_2D, glDefaultObjects->currentRenderState->texture->handle);
    //     }
    // } else {
    //     if (glDefaultObjects->currentRenderState->texture) {
    //         // Deprecated; we choose unlit here, but don't use this anymore anyway.
    //         bindShader(glDefaultObjects->texturedShaderUnlit);
    //         GL_ERROR_EXIT(45)
    //         glActiveTexture(GL_TEXTURE0);
    //         GL_ERROR_EXIT(46)
    //         glBindTexture(GL_TEXTURE_2D, glDefaultObjects->currentRenderState->texture->handle);
    //         GL_ERROR_EXIT(47)
    //     } else {
    //         bindShader(glDefaultObjects->singleColorShader);
    //         glUniform4fv(1, 1, (float*) &glDefaultObjects->currentRenderState->foregroundColor);
    //
    //     }
    // }
    //
    // if (glDefaultObjects->currentRenderState->lightingOn) {
    //     glUniform3fv(10, 1, (float *) &lightDirection);
    //     GL_ERROR_EXIT(123)
    // } else {
    //     glUniform1i(13, 0);
    // }
    //
    //
    // // Tinting
    // glUniform4fv(16, 1, (float*) &glDefaultObjects->currentRenderState->tint);
    // GL_ERROR_EXIT(124)
    //
    // // Flip UVs
    // glUniform1i(20, glDefaultObjects->currentRenderState->flipUvs ? 1 : 0);
    // GL_ERROR_EXIT(125)
    //
    // // UV Panning
    // glUniform1f(22, glDefaultObjects->currentRenderState->panUVS.x);
    // glUniform1f(23, glDefaultObjects->currentRenderState->panUVS.y);
    // GL_ERROR_EXIT(126)
    //
    // // Manipulate the uvs for tiling.
    // if (glDefaultObjects->currentRenderState->tilingOn) {
    //     //glUniform1i(20, 0);
    //     float width_uv = (float)  glDefaultObjects->currentRenderState->tileData.tileWidth / (float) glDefaultObjects->currentRenderState->texture->bitmap->width ;
    //     float height_uv = (float) glDefaultObjects->currentRenderState->tileData.tileHeight / (float) glDefaultObjects->currentRenderState->texture->bitmap->height;
    //     int tileX = glDefaultObjects->currentRenderState->tileData.tileX;
    //     int tileY = glDefaultObjects->currentRenderState->tileData.tileY;
    //     float tileOffsetX = (float) glDefaultObjects->currentRenderState->tileData.tileOffsetX / (float) glDefaultObjects->currentRenderState->texture->bitmap->width;
    //     float tileOffsetY = (float) glDefaultObjects->currentRenderState->tileData.tileOffsetY / (float) glDefaultObjects->currentRenderState->texture->bitmap->height;
    //     float uvs[] = {
    //             tileOffsetX + (tileX * width_uv) , ( 1 - tileOffsetY - (tileY * height_uv)),
    //             tileOffsetX + (tileX * width_uv), (1- tileOffsetY - (tileY * height_uv) - height_uv),
    //             tileOffsetX + (tileX * width_uv) + width_uv,  (1- tileOffsetY -  (tileY * height_uv)- height_uv),
    //             tileOffsetX + (tileX * width_uv) + width_uv, (1 - tileOffsetY - (tileY * height_uv)),
    //
    //     };
    //     glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    //     glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
    // }
    //
    // if (glDefaultObjects->currentRenderState->shadows) {
    //     // Render everything twice, first into the shadowmap buffer here
    //     glViewport(0, 0, 1024, 1024);
    //     prepareShadowMapTransformationMatrices();
    //     glBindFramebuffer(GL_FRAMEBUFFER, glDefaultObjects->shadowMapFramebuffer);
    //     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    //     glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //     glViewport(0, 0, window_width, window_height);
    // }
    //
    // prepareTransformationMatrices();
    //
    // glUniform1f(21, glDefaultObjects->currentRenderState->uvScale);
    //
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    // GL_ERROR_EXIT(344)
    //
    //
    // if (glDefaultObjects->currentRenderState->tilingOn) {
    //     glUniform1i(20, 0);
    //     float uvs[] = {
    //             0, 1,
    //             0, 0,
    //             1, 0,
    //             1, 1,
    //
    //     };
    //     glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    //     glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
    // }
    //
    //
    //
    // glBindTexture(GL_TEXTURE_2D, 0);
    // glBindVertexArray(0);

}

void StatefulRenderer::lightingOn() {
    throw std::runtime_error("Not implemented");
}

void StatefulRenderer::lightingOff() {
    throw std::runtime_error("Not implemented");
}

void StatefulRenderer::wireframeOn(float lineWidth) {
    glLineWidth(lineWidth);
    glPolygonOffset(1, 1);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glEnable(GL_POLYGON_OFFSET_FILL);
}

void StatefulRenderer::wireframeOff() {
    glLineWidth(1.0f);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glPolygonOffset(0, 0);
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void StatefulRenderer::flipUvs(bool val) {
    throw std::runtime_error("Not implemented");
}

void StatefulRenderer::forceShader(Shader *shader) {
}
