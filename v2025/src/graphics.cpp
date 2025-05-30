#ifdef USE_LEGACY_WIN32_RAW

#include <Windows.h>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include <fcntl.h>
#include <inttypes.h>
#include <filesystem>
#include <sstream>
#include <GL/glew.h>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <engine/io/base64.h>
#include <engine/game/SceneNode.h>
#include <engine/graphics/ErrorHandling.h>

GLuint createShadowMapFramebuffer();


void prepareShadowMapTransformationMatrices();
void prepareShadowMapTransformationMatrices(DrawCall& dc);
DrawCall createMeshDrawCall();
void bindShader(Shader* shader);

void debugDummyDrawSkeletalMesh(Mesh *pMesh);





static glm::vec3 lightDirection{0.6, -2, -0.5};

AABB getMovedABBByLocation(AABB sourceAABB, glm::vec3 location) {
    AABB updatedAABB;
    updatedAABB.minX = location.x + sourceAABB.minX;
    updatedAABB.minY = location.y + sourceAABB.minY;
    updatedAABB.minZ = location.z + sourceAABB.minZ;
    updatedAABB.maxX = location.x + sourceAABB.maxX;
    updatedAABB.maxY = location.y + sourceAABB.maxY;
    updatedAABB.maxZ = location.z + sourceAABB.maxZ;
    return updatedAABB;
}

/**
* This is just an "empty" VAO as
* all skybox rendering is done with a fullscreen quad
* where the vertices are within the sky.vert shader.
* We just need a VAO to trigger the vertex-fetch stage
* in our draw call. It will send empty vertices 4 times for a quad.
*
*/
GLuint createSkyboxVAO() {
    GLuint vao;
    glGenVertexArrays(1, &vao);

    return vao;

}

void initDefaultGLObjects() {
    // auto texturedLitVert = readFile("../assets/shaders/textured_lit.vert");
    auto texturedUnlitVert = readFile("../assets/shaders/textured_unlit.vert");
    // auto texturedLitFrag = readFile("../assets/shaders/textured_lit.frag");
    auto texturedUnlitFrag = readFile("../assets/shaders/textured_unlit.frag");
    //
    // auto singleColLitVert = readFile("../assets/shaders/singlecol_lit.vert");
    // auto singleColUnlitVert = readFile("../assets/shaders/singlecol_unlit.vert");
    // auto singleColLitFrag = readFile("../assets/shaders/singlecol_lit.frag");
    // auto singleColUnlitFrag = readFile("../assets/shaders/singlecol_unlit.frag");
    //
    // auto vsrc_skinned = readFile("../assets/shaders/vshader_skinned.glsl");
    // auto vsrc_instanced= readFile("../assets/shaders/vshader_instanced.glsl");
    // auto spriteShaderVertexSource = readFile("../assets/shaders/sprite_vshader.glsl");
    // auto spriteFragmentSource = readFile("../assets/shaders/sprite_fragment_shader.glsl");
    // auto fsrc = readFile("../assets/shaders/fshader_single_color.glsl");
    // auto fsrc_instanced = readFile("../assets/shaders/fshader_single_color_instanced.glsl");
    // auto fsrc_skinned = readFile("../assets/shaders/fshader_single_color_skinned.glsl");
    // auto tfsrc = readFile("../assets/shaders/fshader_diffuse_texture.glsl");
    // auto tfsrc_instanced = readFile("../assets/shaders/fshader_diffuse_texture_instanced.glsl");
    // auto skyboxvs = readFile("../assets/shaders/sky.vert");
    // auto skyboxfs = readFile("../assets/shaders/sky.frag");
    auto gridVertSource = readFile("../assets/shaders/grid.vert");
    auto gridFragSource = readFile("../assets/shaders/grid.frag");
    // auto gridVertPostProcessSource = readFile("../assets/shaders/grid_postprocess.vert");
    // auto gridFragPostProcessSource = readFile("../assets/shaders/grid_postprocess.frag");
    auto shadowMapVertSource = readFile("../assets/shaders/shadow_map.vert");
    auto shadowMapFragSource = readFile("../assets/shaders/shadow_map.frag");

    glDefaultObjects = new GLDefaultObjects();
    glDefaultObjects->shadowMapShader = new Shader();
    glDefaultObjects->singleColorShader = new Shader();
    glDefaultObjects->singleColorShaderInstanced = new Shader();
    glDefaultObjects->skinnedShader = new Shader();
    glDefaultObjects->texturedSkinnedShader = new Shader();
    glDefaultObjects->texturedShaderLit = new Shader();
    glDefaultObjects->texturedShaderUnlit = new Shader();
    glDefaultObjects->spriteShader = new Shader();
    glDefaultObjects->skyboxShader = new Shader();
    glDefaultObjects->gridShader = new Shader();
    glDefaultObjects->gridPostProcessShader = new Shader();
    glDefaultObjects->instancedShader = new Shader();
    glDefaultObjects->quadVAO = createQuadVAO();
    glDefaultObjects->textQuadVAO = createQuadVAO(PlanePivot::topleft);
    glDefaultObjects->shadowMapFramebuffer = createShadowMapFramebuffer();
    glDefaultObjects->currentRenderState = new RenderState();
    glDefaultObjects->currentRenderState->foregroundColor = {0.9, 0.9, 0.9, 1};

    // createShader(singleColLitVert, singleColLitFrag, glDefaultObjects->singleColorShader);
    //createShader(singleColUnlitVert, singleColUnlitFrag, glDefaultObjects->singleColorShader);
    // createShader(vsrc_skinned, fsrc_skinned, glDefaultObjects->skinnedShader);
    // createShader(vsrc_skinned, tfsrc, glDefaultObjects->texturedSkinnedShader);
    // createShader(texturedLitVert, texturedLitFrag, glDefaultObjects->texturedShaderLit);
    createShader(texturedUnlitVert, texturedUnlitFrag, glDefaultObjects->texturedShaderUnlit);
    // createShader(skyboxvs, skyboxfs, glDefaultObjects->skyboxShader);
    // createShader(spriteShaderVertexSource, spriteFragmentSource, glDefaultObjects->spriteShader);
    // createShader(vsrc_instanced, tfsrc_instanced, glDefaultObjects->instancedShader);
    // createShader(vsrc_instanced, fsrc_instanced, glDefaultObjects->singleColorShaderInstanced);
    createShader(gridVertSource, gridFragSource, glDefaultObjects->gridShader);
    // createShader(gridVertPostProcessSource, gridFragPostProcessSource, glDefaultObjects->gridPostProcessShader);
    createShader(shadowMapVertSource, shadowMapFragSource, glDefaultObjects->shadowMapShader);

    glDefaultObjects->gridFBO = createFrameBuffer(scaled_width/2, scaled_height/2);
    glDefaultObjects->defaultUICamera = new Camera();
    glDefaultObjects->defaultUICamera->type = CameraType::Ortho;
    glDefaultObjects->defaultUICamera->updateLocation({0, 0, 2});
    glDefaultObjects->skyboxVAO = createSkyboxVAO();

    srand(time(NULL));
}



std::unique_ptr<Texture> createShadowMapTexture(int width, int height) {
    auto err = glGetError();

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, target->handle());
     glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, width, height);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // These are good settings for a shadow map, to avoid
    // repeating shadows outside of the light frustum.
    // It may not be ideal for non-shadow-map purposes.
     float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
     glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


    err = glGetError();
    if (err != 0) {
        printf("texture gl error: %d\n", err);
        exit(1);
    }


    auto bitmap = new Bitmap();
    bitmap->pixels = nullptr;
    bitmap->width = width;
    bitmap->height = height;
    auto target = std::make_unique<Texture>(textureHandle, bitmap);
    glBindTexture(GL_TEXTURE_2D, 0);

    return target;
}

void clearDepthBuffer() {
    GLfloat d = 1;
    glClearBufferfv(GL_DEPTH, 0, &d);
}

std::unique_ptr<FrameBuffer> createShadowMapFramebufferObject(glm::vec2 size) {
    GLuint handle;
    glGenFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);

    auto texture = createShadowMapTexture(size.x,  size.y);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->handle, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("framebuffer completeness check failure");
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    auto fbo = std::make_unique<FrameBuffer>();
    fbo->handle = handle;
    fbo->texture = std::move(texture);
    return fbo;

}

GLuint createShadowMapFramebuffer() {
    GLuint handle;
    glGenFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);

    glDefaultObjects->shadowMap = createShadowMapTexture(1024,  1024);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glDefaultObjects->shadowMap->handle, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("framebuffer completeness check failure");
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return handle;
}

glm::mat4 projectionMatrixForShadowmap(Camera* cam) {
    if (cam->type == CameraType::Ortho) {
        return glm::ortho<float>(-28, 28, -28, 28, 1.0f, 43.0f);
    }

    return glm::perspectiveFov<float>(glm::radians(50.0f), glDefaultObjects->shadowMap->bitmap->width, glDefaultObjects->shadowMap->bitmap->height, 1.0f, 10);

}

glm::mat4 projectionMatrixForCamera(Camera * cam) {
    if (cam->type == CameraType::Ortho) {
        return glm::ortho<float>(0, scaled_width, 0, scaled_height, 0.1f, 200);
    }else if (cam->type == CameraType::OrthoGameplay) {
        return glm::ortho<float>(-10, 10, -8, 8, 0.1f, 400);
    } else {
        if (glDefaultObjects->currentRenderState->renderToFrameBuffer != nullptr) {
            return glm::perspectiveFov<float>(glm::radians(50.0f), glDefaultObjects->currentRenderState->renderToFrameBuffer->texture->bitmap->width, glDefaultObjects->currentRenderState->renderToFrameBuffer->texture->bitmap->height, 0.1f, 1000);
        } else {
            return glm::perspectiveFov<float>(glm::radians(50.0f), window_width, window_height, 0.1f, 1000);
        }
    }
}

glm::mat4 viewMatrixForCamera(Camera* cam) {
   return lookAt((cam->location), (cam->lookAtTarget), glm::vec3(0, 1,0));

};



GridData * createGrid(int lines) {
    auto vao = createGridVAO(lines);
    auto gd = new GridData();
    gd->numLines = lines;
    gd->vao = vao;
    return gd;
}



std::unique_ptr<Bitmap> loadBitmap(const char* fileName) {
    auto bitmap = std::make_unique<Bitmap>();
    if (isPngFile(fileName) || isJpgFile(fileName)) {
        int imageChannels;
        int w, h;
        auto pixels = stbi_load(
                fileName, &w, &h,
                &imageChannels,
                4);
        bitmap->pixels = pixels;
        bitmap->width = w;
        bitmap->height = h;
    }
    else {
        int file;
        _sopen_s( &file, fileName, _O_RDONLY, _SH_DENYNO, 0 );
        if (file == -1) {
            exit(1);
        }
        _lseek(file, 0, SEEK_SET);

        BITMAP_FILE bmf;
        _read(file, &bmf.bmfileHeader, sizeof(BITMAPFILEHEADER));

        if (bmf.bmfileHeader.bfType != 0x4D42) {
            _lclose(file);
            exit(2);
        }
        _read(file, &bmf.bminfoHeader, sizeof(BITMAPINFOHEADER));
        if (bmf.bminfoHeader.biBitCount != 32) {
            _lclose(file);
            exit(3);
        }

        _lseek(file, bmf.bmfileHeader.bfOffBits, SEEK_SET);
        bitmap->pixels = (uint8_t*) malloc(bmf.bminfoHeader.biSizeImage);
        _read(file, bitmap->pixels, bmf.bminfoHeader.biSizeImage);
        _close(file);
        bitmap->width = bmf.bminfoHeader.biWidth;
        bitmap->height = bmf.bminfoHeader.biHeight;
    }

    return bitmap;

}



void beginBatch() {
    // Not allowed to stack batch modes.
    assert(glDefaultObjects->inBatchMode == false);
    glDefaultObjects->inBatchMode = true;
    glBindVertexArray(glDefaultObjects->quadVAO);


}

Result createComputeShader(const std::string& source, Shader* target) {
    GLuint cshader = glCreateShader(GL_COMPUTE_SHADER);
    const GLchar* vssource_char = source.c_str();
    glShaderSource(cshader, 1, &vssource_char, NULL);
    glCompileShader(cshader);
    GLint compileStatus;
    glGetShaderiv(cshader, GL_COMPILE_STATUS, &compileStatus);
    if (GL_FALSE == compileStatus) {
        OutputDebugString("Error while compiling the compute shader\n");

        GLint logSize = 0;
        glGetShaderiv(cshader, GL_INFO_LOG_LENGTH, &logSize);
        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(cshader, logSize, &logSize, &errorLog[0]);
        //    result.errorMessage = errorLog.data();
        char buf[512];
        sprintf(buf, "vshader error: %s", errorLog.data());
        printf(buf);
        OutputDebugStringA(buf);
        glDeleteShader(cshader);
        //  return result;
        return {false, {buf}};

    }

    GLuint p = glCreateProgram();
    glAttachShader(p, cshader);
    glLinkProgram(p);

    glGetProgramiv(p, GL_LINK_STATUS, &compileStatus);

    if (GL_FALSE == compileStatus) {
        OutputDebugStringA("Error during shader linking\n");
        GLint maxLength = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(p, maxLength, &maxLength, &infoLog[0]);
        OutputDebugStringA(infoLog.data());
        glDeleteProgram(p);
        glDeleteShader(cshader);
        return {false, {{infoLog.data()}}};

    }

    glDeleteShader(cshader);

    target->handle = p;

    return {true ,{}};
}

Result createShader(const std::string &vsrc, const std::string &fsrc, Shader* target) {
    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vssource_char = vsrc.c_str();
    glShaderSource(vshader, 1, &vssource_char, NULL);
    glCompileShader(vshader);
    GLint compileStatus;
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &compileStatus);
    if (GL_FALSE == compileStatus) {
        OutputDebugString("Error while compiling the vertex shader\n");

        GLint logSize = 0;
        glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &logSize);
        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(vshader, logSize, &logSize, &errorLog[0]);
        //    result.errorMessage = errorLog.data();
        char buf[512];
        sprintf(buf, "vshader error: %s", errorLog.data());
        printf(buf);
        OutputDebugStringA(buf);
        glDeleteShader(vshader);
        //  return result;
        return {false, {buf}};

    }


    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fssource_char = fsrc.c_str();
    glShaderSource(fshader, 1, &fssource_char, NULL);
    glCompileShader(fshader);

    glGetShaderiv(fshader, GL_COMPILE_STATUS, &compileStatus);
    if (GL_FALSE == compileStatus) {
        GLint logSize = 0;
        glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &logSize);
        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(fshader, logSize, &logSize, &errorLog[0]);
        //   result.errorMessage = errorLog.data();
        printf("fragment shader error: %s", errorLog.data());
        glDeleteShader(fshader);
        return {false, {{errorLog.data()}}};

    }

    GLuint p = glCreateProgram();
    glAttachShader(p, vshader);
    glAttachShader(p, fshader);
    glLinkProgram(p);

    glGetProgramiv(p, GL_LINK_STATUS, &compileStatus);

    if (GL_FALSE == compileStatus) {
        OutputDebugStringA("Error during shader linking\n");
        GLint maxLength = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(p, maxLength, &maxLength, &infoLog[0]);
        OutputDebugStringA(infoLog.data());
        glDeleteProgram(p);
        glDeleteShader(vshader);
        glDeleteShader(fshader);
        return {false, {{infoLog.data()}}};

    }

    GLenum err = glGetError();
    if (err != 0) {
        return {false, {{std::to_string(err)}}};
    }

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    target->handle = p;

    return {true ,{}};
}




GLuint createQuadVAO(PlanePivot pivot) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // This is the case for center pivot (default):
    std::vector<float> positions = {
            -0.5, 0.5, 0,       // top left
            -0.5, -0.5, 0,      // bottom left
            0.5, -0.5, 0,       // bottom right
            0.5, 0.5, 0       // top right
    };

    if (pivot == PlanePivot::topleft) {
        positions.clear();
        positions = {
                0, 1, 0,       // top left
                0, 0, 0,      // bottom left
                1, 0, 0,       // bottom right
                1, 1, 0       // top right
        };
    }

    // TODO other pivot variants


    GLuint indices[] = {
            0, 1, 2,
            0, 2, 3
    };

    glGenBuffers(1, &glDefaultObjects->quadPosBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, 12 * 4, positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // UVS
    {
        float uvs[] = {
                0, 1,
                0, 0,
                1, 0,
                1, 1,

        };
        glGenBuffers(1, &glDefaultObjects->quadUVBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
        glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
    }

    glGenBuffers(1, &glDefaultObjects->quadIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glDefaultObjects->quadIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, indices, GL_STATIC_DRAW);

    auto err = glGetError();
    if (err != 0) {
        printf("gl Error");
    }
    glBindVertexArray(0);
    return vao;
}

glm::vec3 toGlm(Vec3 v) {
    return {v.x, v.y, v.z};
}

glm::vec4 toGlm4(Vec4 v) {
    return {v.x, v.y, v.z, v.w};
}

/**
 * This method is a convenient shortcut to prepare all
 * transformation matrices before a draw call.
 * (or maybe better, a series of draw calls).
 * All necessary information is drawn from the current render state.
 */
void prepareTransformationMatrices() {

    if (glDefaultObjects->currentRenderState->useWorldMatrix) {
        prepareTransformationMatrices(glDefaultObjects->matWorld,
                                      glDefaultObjects->currentRenderState->camera,
                                      glDefaultObjects->currentRenderState->shadowMapCamera,
                                      glDefaultObjects->currentRenderState->shadows);
    } else {
        prepareTransformationMatrices((glDefaultObjects->currentRenderState->location),
                                      (glDefaultObjects->currentRenderState->scale),
                                      (glDefaultObjects->currentRenderState->rot),
                                      glDefaultObjects->currentRenderState->camera,
                                      glDefaultObjects->currentRenderState->shadowMapCamera,
                                      glDefaultObjects->currentRenderState->shadows);
    }


    return;

    // Deprecated
    // Just keep for now
    {
        using namespace glm;
        // Object to world transformation
        mat4 mattrans = translate(mat4(1), glDefaultObjects->currentRenderState->location);
        mat4 matscale = scale(mat4(1), glDefaultObjects->currentRenderState->scale);
        mat4 matrotX = glm::rotate(mat4(1), glm::radians(glDefaultObjects->currentRenderState->rot.x), {1, 0, 0});
        mat4 matrotY = glm::rotate(mat4(1), glm::radians(glDefaultObjects->currentRenderState->rot.y), {0, 1, 0});
        mat4 matrotZ = glm::rotate(mat4(1), glm::radians(glDefaultObjects->currentRenderState->rot.z), {0, 0, 1});
        mat4 matworld = mattrans * matrotX * matrotY * matrotZ * matscale;

        glUniformMatrix4fv(4, 1, GL_FALSE, value_ptr(matworld));

        glUniformMatrix4fv(5, 1, GL_FALSE,
                           value_ptr(viewMatrixForCamera(glDefaultObjects->currentRenderState->camera)));
        glUniformMatrix4fv(6, 1, GL_FALSE,
                           value_ptr(projectionMatrixForCamera(glDefaultObjects->currentRenderState->camera)));


        if (glDefaultObjects->currentRenderState->shadows) {
            mat4 directionalLightViewProjection =
                    projectionMatrixForShadowmap(glDefaultObjects->currentRenderState->shadowMapCamera) *
                    viewMatrixForCamera(glDefaultObjects->currentRenderState->shadowMapCamera);
            glUniformMatrix4fv(7, 1, GL_FALSE, value_ptr(directionalLightViewProjection));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, glDefaultObjects->shadowMap->handle);
        }


    }
}


void foregroundColor(glm::vec4 col) {
    glDefaultObjects->currentRenderState->foregroundColor = {col.x, col.y, col.z, col.w};
}

/**
 * This updates the buffer of the quadVAO and renders the contents.
 *
 */
void endBatch() {
    bindShader(glDefaultObjects->singleColorShader);
    glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, glDefaultObjects->vertices.size() *3 * 4, glDefaultObjects->vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glDefaultObjects->quadIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, glDefaultObjects->indices.size() * 4, glDefaultObjects->indices.data(), GL_DYNAMIC_DRAW);

    Mat4 matworld = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    Mat4 matproj = orthoProjection(-1, 1, -1, 1, 0.1, 200);

    glUniformMatrix4fv(	4, 1, GL_FALSE, (float*) &matworld);
    glUniformMatrix4fv(	6, 1, GL_FALSE, (float*) &matproj);
    glUniform4fv(1, 1, (float*) &glDefaultObjects->currentRenderState->foregroundColor);

    glDrawElements(GL_TRIANGLES, glDefaultObjects->indices.size(), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glDefaultObjects->vertices.clear();
    glDefaultObjects->indices.clear();
    glDefaultObjects->inBatchMode = false;
}

void scale(glm::vec3 val) {
    glDefaultObjects->currentRenderState->scale = val;
}

void rotation(glm::vec3 rot) {
    glDefaultObjects->currentRenderState->rot = rot;
}

void rotation(glm::mat4* rotMatrix) {
    glDefaultObjects->currentRenderState->rotMatrix = rotMatrix;
}


void location(glm::vec3 loc) {
    glDefaultObjects->currentRenderState->location = loc;
}


GLenum toGLColorFormat(ColorFormat cf) {
    if (cf == ColorFormat::RGBA) return GL_SRGB8_ALPHA8;
    return GL_R;
}

std::unique_ptr<Texture> createTextureFromBitmap(Bitmap *bm, ColorFormat colorFormat) {
    GLuint handle;
    glGenTextures(1, &handle);

    glBindTexture(GL_TEXTURE_2D, handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_SRGB8_ALPHA8,
                 bm->width, bm->height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE, bm->pixels);

    auto target = std::make_unique<Texture>();
    target->handle = handle;
    target->bitmap = bm;
    return target;
}

/**
* Expects a directory which contains 6 files, named top, bottom, back, left, right, front.
* The filetype (extension can be given)
*/
std::unique_ptr<Texture> createCubeMapTextureFromDirectory(const std::string &dirName, ColorFormat colorFormat, const std::string& fileType) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);


    std::vector<std::string> faces = {
            "right." + fileType,
            "left." + fileType,
            "top." + fileType,
            "bottom." + fileType,
            "back." + fileType,
            "front." + fileType,
    };

    for (unsigned int i = 0; i < faces.size(); i++)
    {

        auto fileName = (dirName + "/" + faces[i]);
        auto bm = loadBitmap(fileName.c_str());

        if (bm)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_SRGB8_ALPHA8, bm->width, bm->height, 0,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         bm->pixels
            );
            // TODO
//            stbi_image_free(data);
        }
        else
        {
            throw std::runtime_error("Cubemap texture failed to load at path: " + dirName);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    auto target = std::make_unique<Texture>();
    target->handle = textureID;
    return target;
}

glm::mat4 getWorldMatrixFromGlobalState() {

    using namespace glm;

    // Object to world transformation
    mat4 mattrans = translate(mat4(1), glDefaultObjects->currentRenderState->location);
    mat4 matscale = glm::scale(mat4(1),glDefaultObjects->currentRenderState->scale);
    mat4 matworld = glm::mat4(1);

    // For rotation we check if we have a rotation matrix set.
    if (glDefaultObjects->currentRenderState->rotMatrix) {
        matworld = mattrans * (*glDefaultObjects->currentRenderState->rotMatrix) * matscale;

    } else {
        mat4 matrotX = glm::rotate(mat4(1), glm::radians(glDefaultObjects->currentRenderState->rot.x), {1, 0, 0} );
        mat4 matrotY = glm::rotate(mat4(1), glm::radians(glDefaultObjects->currentRenderState->rot.y), {0, 1, 0} );
        mat4 matrotZ = glm::rotate(mat4(1), glm::radians(glDefaultObjects->currentRenderState->rot.z), {0, 0, 1} );
        matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;
    }

    return matworld;

}

std::unique_ptr<Texture> createTextureFromFile(const std::string &fileName, GLenum colorFormat, GLint internalColorFormat) {



}

void clearGLBuffers() {
    auto cc = glDefaultObjects->currentRenderState->clearColor;
    glClearColor(cc.r, cc.g, cc.b, cc.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, glDefaultObjects->shadowMapFramebuffer);
    clearDepthBuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void bindTexture(Texture *tex) {
    glDefaultObjects->currentRenderState->texture = tex;
}

void bindNormalMap(Texture* tex, int unit) {
    glDefaultObjects->currentRenderState->normalMap = tex;
    glDefaultObjects->currentRenderState->normalMapTextureUnit = unit;

}

void bindSkyboxTexture(Texture* tex) {
    glDefaultObjects->currentRenderState->texture = nullptr;
    glDefaultObjects->currentRenderState->skyboxTexture = tex;
}

void unbindSkyboxTexture() {
    glDefaultObjects->currentRenderState->skyboxTexture = nullptr;
}


void drawMeshCallExecution(DrawCall drawCall) {
    glBindVertexArray(drawCall.mesh->vao);
    if (drawCall.texture) {
        bindShader(glDefaultObjects->instancedShader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, drawCall.texture->handle);
    } else {
        bindShader(glDefaultObjects->singleColorShaderInstanced);
        glUniform4fv(1, 1, (float*) &drawCall.foregroundColor);
    }

    if (drawCall.lightingOn) {
        glUniform1i(13, 1);
    } else {
        glUniform1i(13, 0);
    }

    if (drawCall.flipUvs) {
        glUniform1i(20, 1);
    } else {
        glUniform1i(20, 0);
    }

    glUniform1f(21, drawCall.uvScale);

    //glUniform4fv(16, 1, (float*) &drawCall.tint);
    glUniform1f(15, drawCall.overrideAlpha);


    glBindBuffer(GL_ARRAY_BUFFER, drawCall.mesh->instanceOffsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * drawCall.instanceOffsets.size(),
                     drawCall.instanceOffsets.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, drawCall.mesh->instanceColorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * drawCall.instanceColors.size(),
                 drawCall.instanceColors.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, drawCall.mesh->instanceTintVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * drawCall.instanceTints.size(),
                 drawCall.instanceTints.data(), GL_DYNAMIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, drawCall.mesh->instanceMatrixVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * drawCall.instanceMatrices.size(),
                 drawCall.instanceMatrices.data(), GL_DYNAMIC_DRAW);

    GL_ERROR_EXIT(99)



    if (drawCall.shadows && drawCall.shadowPass) {
        // Render everything twice, first into the shadowmap buffer here
        glViewport(0, 0, 1024, 1024);

        glBindFramebuffer(GL_FRAMEBUFFER, glDefaultObjects->shadowMapFramebuffer);
        prepareShadowMapTransformationMatrices(drawCall);
        GL_ERROR_EXIT(101)
        // We are using instanced rendering now all the time even if only 1 instance is rendered.
        glDrawElementsInstanced(GL_TRIANGLES, drawCall.mesh->numberOfIndices, drawCall.mesh->indexDataType, nullptr, drawCall.instances);

        //glDrawElements(GL_TRIANGLES, drawCall.mesh->numberOfIndices, drawCall.mesh->indexDataType, nullptr);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);
    }

    if (!drawCall.shadowPass) {
        prepareTransformationMatrices(drawCall);
        glDrawElementsInstanced(GL_TRIANGLES, drawCall.mesh->numberOfIndices, drawCall.mesh->indexDataType, nullptr, drawCall.instances);

        auto err = glGetError();
        if (err != 0) {
            exit(2);
        }

    }

    glUniform1f(15, 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

/**
 * We only support this in deferred mode for now
 * @param num The number of instances to draw
 */
void drawMeshInstanced(int num) {
    if (glDefaultObjects->currentRenderState->deferred) {
        auto dc = createMeshDrawCall();
        dc.instances = num;
        glDefaultObjects->currentRenderState->drawCalls.push_back(dc);
    }
}

DrawCall createMeshDrawCall() {
    DrawCall dc;
    dc.location = glDefaultObjects->currentRenderState->location;
    dc.rotation = glDefaultObjects->currentRenderState->rot;
    dc.scale = glDefaultObjects->currentRenderState->scale;
    dc.mesh = glDefaultObjects->currentRenderState->mesh;
    dc.texture = glDefaultObjects->currentRenderState->texture;
    dc.foregroundColor = glDefaultObjects->currentRenderState->foregroundColor;
    dc.overrideAlpha = glDefaultObjects->currentRenderState->overrideAlpha;
    dc.flipUvs = glDefaultObjects->currentRenderState->flipUvs;
    dc.shadows = glDefaultObjects->currentRenderState->shadows;
    dc.uvScale = glDefaultObjects->currentRenderState->uvScale;
    dc.camera = glDefaultObjects->currentRenderState->camera;
    dc.shadowMapCamera = glDefaultObjects->currentRenderState->shadowMapCamera;
    dc.instanceOffsets = glDefaultObjects->currentRenderState->instanceOffsets;
    dc.instanceMatrices = glDefaultObjects->currentRenderState->instanceMatrices;
    dc.instanceColors = glDefaultObjects->currentRenderState->instanceColors;
    dc.instanceTints = glDefaultObjects->currentRenderState->instanceTints;
    return dc;
}


void drawMeshSimple() {
    glBindVertexArray(glDefaultObjects->currentRenderState->mesh->vao);
    if (glDefaultObjects->currentRenderState->texture) {
        bindShader(glDefaultObjects->texturedShaderUnlit);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glDefaultObjects->currentRenderState->texture->handle);
    } else {
        bindShader(glDefaultObjects->singleColorShader);
        glUniform4fv(1, 1, (float*) &glDefaultObjects->currentRenderState->foregroundColor);
    }

    if (glDefaultObjects->currentRenderState->lightingOn) {
        glUniform1i(13, 1);
    } else {
        glUniform1i(13, 0);
    }

    if (glDefaultObjects->currentRenderState->flipUvs) {
        glUniform1i(20, 1);
    } else {
        glUniform1i(20, 0);
    }

    glUniform1f(21, glDefaultObjects->currentRenderState->uvScale);

    if (glDefaultObjects->currentRenderState->shadows) {
        // Render everything twice, first into the shadowmap buffer here
        glViewport(0, 0, 1024, 1024);
        glBindFramebuffer(GL_FRAMEBUFFER, glDefaultObjects->shadowMapFramebuffer);
        prepareShadowMapTransformationMatrices();
        glDrawElements(GL_TRIANGLES, glDefaultObjects->currentRenderState->mesh->numberOfIndices, glDefaultObjects->currentRenderState->mesh->indexDataType, nullptr);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);
    }

    prepareTransformationMatrices();
    if (glGetError() != 0) {
        exit(1);
    }

    glDrawElements(GL_TRIANGLES, glDefaultObjects->currentRenderState->mesh->numberOfIndices, glDefaultObjects->currentRenderState->mesh->indexDataType, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

/**
* Draws a skybox as a fullscreen quad
*/
void drawSkybox() {
    glBindVertexArray(glDefaultObjects->skyboxVAO);
    bindShader(glDefaultObjects->skyboxShader);
    //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glUniformMatrix4fv( 0, 1, GL_FALSE, value_ptr(viewMatrixForCamera(glDefaultObjects->currentRenderState->camera)));
    glUniformMatrix4fv( 1, 1, GL_FALSE, value_ptr(projectionMatrixForCamera(glDefaultObjects->currentRenderState->camera)));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, glDefaultObjects->currentRenderState->skyboxTexture->handle);

    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDepthMask(GL_TRUE);


}

void drawMesh() {
    if (glDefaultObjects->currentRenderState->deferred) {
        // Only create a draw call here.
        auto dc = createMeshDrawCall();
        glDefaultObjects->currentRenderState->drawCalls.push_back(dc);
        return;
    }


    glBindVertexArray(glDefaultObjects->currentRenderState->mesh->vao);

    static int i = 0;
    i++;
    auto glerr = glGetError();
    if (glerr != 0) {
        printf("i: %d\n", i);

    }

    if (glDefaultObjects->currentRenderState->texture) {
        if (glDefaultObjects->currentRenderState->skinnedDraw) {
            bindShader(glDefaultObjects->texturedSkinnedShader);
        } else {
            bindShader(glDefaultObjects->texturedShaderLit);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glDefaultObjects->currentRenderState->texture->handle);
        GL_ERROR_EXIT(9800);
    } else {
        if (glDefaultObjects->currentRenderState->skinnedDraw) {
            bindShader(glDefaultObjects->skinnedShader);
        } else {
            bindShader(glDefaultObjects->singleColorShader);
        }
        glUniform4fv(1, 1, (float *) &glDefaultObjects->currentRenderState->foregroundColor);
    }


    if (glDefaultObjects->currentRenderState->normalMap) {
        glActiveTexture(GL_TEXTURE0 + glDefaultObjects->currentRenderState->normalMapTextureUnit);
        glBindTexture(GL_TEXTURE_2D, glDefaultObjects->currentRenderState->normalMap->handle);
        GL_ERROR_EXIT(981);
    }

    if (glDefaultObjects->currentRenderState->lightingOn && glDefaultObjects->currentRenderState->currentLight) {
        glUniform1i(13, 1);

        glm::vec3 dir = glDefaultObjects->currentRenderState->currentLight->lookAtTarget - glDefaultObjects->currentRenderState->currentLight->location;
        glUniform3fv(1, 1, glm::value_ptr(dir));
        GLint locLightPos = glGetUniformLocation(glDefaultObjects->texturedShaderLit->handle, "lightPos");
        glUniform3fv(locLightPos, 1, glm::value_ptr(glDefaultObjects->currentRenderState->currentLight->location));

        GLint loc = glGetUniformLocation(glDefaultObjects->texturedShaderLit->handle, "isPointLight");
        if (glDefaultObjects->currentRenderState->currentLight->type == LightType::Point) {
            glUniform1i(loc, 1);
        } else {
            glUniform1i(loc, 0);
        }

        GL_ERROR_EXIT(983);

    } else {
        glUniform1i(13, 0);
    }

    if (glDefaultObjects->currentRenderState->flipUvs) {
        glUniform1i(20, 1);
    } else {
        glUniform1i(20, 0);

    }
    glUniform1f(21, glDefaultObjects->currentRenderState->uvScale);

    prepareTransformationMatrices();

    if (!glDefaultObjects->currentRenderState->depthTest) {
        glDisable(GL_DEPTH_TEST);
    } else {
        glEnable(GL_DEPTH_TEST);
    }


    glDrawElements(GL_TRIANGLES, glDefaultObjects->currentRenderState->mesh->numberOfIndices, glDefaultObjects->currentRenderState->mesh->indexDataType, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);
}

void drawMeshSimple2(const MeshDrawData& drawData) {
    glBindVertexArray(drawData.mesh->vao);
    bindShader(drawData.shader);
    drawData.texture->bindAt(0);
    GL_ERROR_EXIT(88100)

    {
        using namespace glm;

        // Object to world transformation
        mat4 mattrans = translate(mat4(1), drawData.location);
        mat4 matscale = glm::scale(mat4(1),drawData.scale);
        mat4 matworld = glm::mat4(1);

        // For rotation we check if we have a rotation matrix set.
        if (glDefaultObjects->currentRenderState->rotMatrix) {
            matworld = mattrans * (*glDefaultObjects->currentRenderState->rotMatrix) * matscale;

        } else {
            mat4 matrotX = glm::rotate(mat4(1), glm::radians(drawData.rotationEulers.x), {1, 0, 0} );
            mat4 matrotY = glm::rotate(mat4(1), glm::radians(drawData.rotationEulers.y), {0, 1, 0} );
            mat4 matrotZ = glm::rotate(mat4(1), glm::radians(drawData.rotationEulers.z), {0, 0, 1} );
            matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;
        }

        drawData.shader->setMat4Value(matworld, "mat_model");
        drawData.shader->setMat4Value(drawData.camera->getViewMatrix(), "mat_view");
        drawData.shader->setMat4Value(drawData.camera->getProjectionMatrix(), "mat_projection");
        GL_ERROR_EXIT(88101)
    }


    glDrawElements(GL_TRIANGLES, drawData.mesh->numberOfIndices, drawData.mesh->indexDataType, nullptr);
    GL_ERROR_EXIT(88102);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);


}


void drawMeshIntoShadowMap(const MeshDrawData& drawData, Light* directionalLight) {
    glBindVertexArray(drawData.mesh->vao);
    GL_ERROR_EXIT(1234);
    bindShader(drawData.shader);
    GL_ERROR_EXIT(1235);

    glViewport(0, 0, drawData.viewPortDimensions.value().x, drawData.viewPortDimensions.value().y);
    {
        using namespace glm;

        // Object to world transformation
        mat4 mattrans = translate(mat4(1), drawData.location);
        mat4 matscale = glm::scale(mat4(1), drawData.scale);
        mat4 matrotX = glm::rotate(mat4(1), glm::radians(drawData.rotationEulers.x), {1, 0, 0} );
        mat4 matrotY = glm::rotate(mat4(1), glm::radians(drawData.rotationEulers.y), {0, 1, 0} );
        mat4 matrotZ = glm::rotate(mat4(1), glm::radians(drawData.rotationEulers.z), {0, 0, 1} );
        mat4 matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;

        drawData.shader->setMat4Value(matworld, "mat_world");
        drawData.shader->setMat4Value(directionalLight->getShadowViewMatrix(drawData.camera), "mat_view");
        drawData.shader->setMat4Value(directionalLight->getShadowProjectionMatrix(drawData.camera), "mat_projection");
    }
    glDrawElements(GL_TRIANGLES, drawData.mesh->numberOfIndices,
                   drawData.mesh->indexDataType, nullptr);

    glViewport(0, 0, scaled_width, scaled_height);
    glBindVertexArray(0);
}

[[Deprecated("use the method with the MeshDrawData instead")]]
/*
void drawMeshIntoShadowMap(FrameBuffer* shadowMapFBO) {
    glBindVertexArray(glDefaultObjects->currentRenderState->mesh->vao);


    if (glDefaultObjects->currentRenderState->skinnedDraw) {
        bindShader(glDefaultObjects->skinnedShadowMapShader);
    } else {
        bindShader(glDefaultObjects->shadowMapShader);
    }
    glViewport(0, 0, shadowMapFBO->texture->bitmap->width, shadowMapFBO->texture->bitmap->height);

    prepareShadowMapTransformationMatrices();
    glDrawElements(GL_TRIANGLES, glDefaultObjects->currentRenderState->mesh->numberOfIndices,
                   glDefaultObjects->currentRenderState->mesh->indexDataType, nullptr);

    glViewport(0, 0, scaled_width, scaled_height);
    glBindVertexArray(0);
}
*/
void debugDummyDrawSkeletalMesh(Mesh *pMesh) {
    glDefaultObjects->currentRenderState->mesh->indices;
    printf("debug rendering: indices: \n");

    auto indices = glDefaultObjects->currentRenderState->mesh->indices;
    auto positions = glDefaultObjects->currentRenderState->mesh->positions;
    auto boneIndices = glDefaultObjects->currentRenderState->mesh->boneIndices;
    auto boneWeights = glDefaultObjects->currentRenderState->mesh->boneWeights;
    for (int i = 0; i < indices.size(); i++) {
        auto index = indices[i];
        auto pos = positions[index];
        auto a_BoneIndices = boneIndices[index];
        auto a_BoneWeights = boneWeights[index];
        printf("\t");
        printf("index: %u ", index);
        printf("%f/%f/%f", pos.x, pos.y, pos.z);
        printf("\t");
        printf("%d/%d/%d/%d", a_BoneIndices.x, a_BoneIndices.y,
               a_BoneIndices.z, a_BoneIndices.w);


        // Next we simulate the actual vertex shader calculation:
        /**
         *
            vec4 skinnedPosition = vec4(0.0);
            int skinned = 0;

            // Iterate over the 4 influencing bones
            for (int i = 0; i < 4; i++) {
                int boneIndex = a_BoneIndices[i];

                if (boneIndex != -1) {
                    skinned = 1;
                    mat4 boneMatrix = u_BoneMatrices[boneIndex]; // Fetch the bone's matrix
                    skinnedPosition += boneMatrix * vec4(pos, 1.0) * a_BoneWeights[i];
                }
            }

            // No bone influence
            if (skinned == 0) {
                skinnedPosition = vec4(pos, 1);
            }

            gl_Position = mat_projection * mat_view * mat_model* vec4(skinnedPosition);
         */
        glm::vec4 skinnedPosition = glm::vec4(0);
        int skinned = 0;
        for (int b = 0; b < 4; b++) {
            int boneIndex = a_BoneIndices[b];
            if (boneIndex != -1) {
                skinned = 1;
                glm::mat4 boneMatrix = glDefaultObjects->boneMatrices[boneIndex];

                auto c1 = boneMatrix[0];
                auto c2 = boneMatrix[1];
                auto c3 = boneMatrix[2];
                auto c4 = boneMatrix[3];


                skinnedPosition += boneMatrix * glm::vec4(pos, 1.0) * a_BoneWeights[b];
            }
        }

        if (skinned == 0) {
            skinnedPosition = glm::vec4(pos, 1);
        }

        using namespace glm;
        auto location = glDefaultObjects->currentRenderState->location;
        auto scale = glDefaultObjects->currentRenderState->scale;
        auto rotation = glDefaultObjects->currentRenderState->rot;

        mat4 mattrans = translate(mat4(1), location);
        mat4 matscale = glm::scale(mat4(1),scale);
        mat4 matrotX = rotate(mat4(1), glm::radians(rotation.x), {1, 0, 0} );
        mat4 matrotY = rotate(mat4(1), glm::radians(rotation.y), {0, 1, 0} );
        mat4 matrotZ = rotate(mat4(1), glm::radians(rotation.z), {0, 0, 1} );
        mat4 matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;

        auto viewMatrix = viewMatrixForCamera(glDefaultObjects->currentRenderState->camera);
        auto projMatrix = projectionMatrixForCamera(glDefaultObjects->currentRenderState->camera);
        auto gl_Position = projMatrix * viewMatrix * matworld * glm::vec4(skinnedPosition);
        printf("\t");
        printf("%f/%f/%f/%f", gl_Position.x, gl_Position.y, gl_Position.z, gl_Position.w);
        printf("\n");
    }
    printf("-------------------------------------------------------\n");
}


void bindMesh(Mesh *mesh) {
    glDefaultObjects->currentRenderState->mesh = mesh;
}

void bindCamera(Camera *camera) {
    glDefaultObjects->currentRenderState->camera = camera;
}

void bindShadowMapCamera(Camera* camera) {
    glDefaultObjects->currentRenderState->shadowMapCamera = camera;
}

GLuint createGridVAO(int numLines) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    int horizontalDistance = 1;
    int verticalDistance = 1;
    int horizontalLines = numLines;
    int verticalLines = numLines;

    GLuint vboPos;
    auto positions = std::vector<float>((horizontalLines * 2 * 3) + (verticalLines * 2* 3) + 2 * 2 * 3);
    int posIndex = 0;
    int firstH = -horizontalLines / 2 * horizontalDistance;
    int firstV = -verticalLines / 2 * verticalDistance;
    for (int h = 0; h < horizontalLines; h++) {
        // Start point
        positions[posIndex++] = firstH;    // x
        positions[posIndex++] = -0.01;                              // we are on the ground, y = zero
        positions[posIndex++] = firstH + h * horizontalDistance;    // z

        // End point
        positions[posIndex++] = -firstH;                            // x
        positions[posIndex++] = -.01;                              // we are on the ground, y = zero
        positions[posIndex++] = firstH + h * horizontalDistance;    // z

    }

    for (int v = 0; v < verticalLines; v++) {
        // Start point V
        positions[posIndex++] = firstV + v * verticalDistance; ;    // x
        positions[posIndex++] = -.02;                              // we are on the ground, y = zero
        positions[posIndex++] = firstV;   // z

        // End point vertical
        positions[posIndex++] = firstV + v * verticalDistance; ;                            // x
        positions[posIndex++] = -.02;                              // we are on the ground, y = zero
        positions[posIndex++] = -firstV;  // z
    }

    int sizeOfHLines = horizontalLines * 6 * 4;
    int sizeOfVLines = verticalLines * 6 * 4;

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeOfHLines + sizeOfVLines, positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return vao;
}



bool startsWith(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

std::vector<std::string> splittolino(const std::string& theString, const char* delim) {
    std::vector<std::string> resultList;
    char* fileStr = (char*) theString.c_str();
    char* line;
    //memset(line, 0, 200);
    line = strtok(fileStr, delim);

    while (line != nullptr)
    {
        resultList.push_back(line);
        line =  strtok(NULL, delim);
    }

    return resultList;
}

std::vector<glm::vec3> extractPositions(std::vector<std::string> lines) {
    std::vector<glm::vec3> resultList;
    for (auto line : lines) {
        if (startsWith("v ", line.c_str())) {
            auto parts = splittolino(line, " ");
            resultList.push_back({atof(parts[1].c_str()), atof(parts[2].c_str()), atof(parts[3].c_str())});

        }
    }

    return resultList;
}

std::vector<glm::vec3> extractNormals(std::vector<std::string> lines) {
    std::vector<glm::vec3> resultList;
    for (auto line : lines) {
        if (startsWith("vn ", line.c_str())) {
            auto parts = splittolino(line, " ");
            resultList.push_back({atof(parts[1].c_str()), atof(parts[2].c_str()), atof(parts[3].c_str())});

        }
    }

    return resultList;
}

std::vector<glm::vec2> extractUVs(std::vector<std::string> lines) {
    std::vector<glm::vec2> resultList;
    for (auto line : lines) {
        if (startsWith("vt ", line.c_str())) {
            auto parts = splittolino(line, " ");
            resultList.push_back({atof(parts[1].c_str()), atof(parts[2].c_str())});

        }
    }

    return resultList;
}

std::vector<glm::vec3> extractFacesData(std::vector<std::string> lines, int facesIndex) {
    std::vector<glm::vec3> resultList;
    for (auto line : lines) {
        if (startsWith("f ", line.c_str())) {
            auto parts = splittolino(line, " ");
            float fvec[3];
            for (int i = 1; i<4;i++) {
                auto p = parts[i];
                auto subParts = splittolino(p, "/");
                fvec[i-1] = atof(subParts[facesIndex].c_str());
            }
            resultList.push_back({fvec[0], fvec[1], fvec[2]});

        }
    }

    return resultList;
}

// Put in the correct sequence
std::vector<glm::vec3> correctV3Data(std::vector<glm::vec3> dataIn, std::vector<glm::vec3> facesList, std::vector<glm::vec3> sourceFaceList, int targetSize) {
    std::vector<glm::vec3> correctedList;
    correctedList.resize(targetSize);
    int index = 0;
    for (auto fn : facesList) {
        int sourceIndex0 = sourceFaceList.at(index).x-1;
        int sourceIndex1 = sourceFaceList.at(index).y-1;
        correctedList[fn.x-1] = dataIn[sourceIndex0];
        correctedList[fn.y-1] = dataIn[sourceIndex1];
        correctedList[fn.z-1] = dataIn[sourceFaceList[index].z -1];
        index++;
    }
    return correctedList;
}

std::vector<glm::vec2> correctV2Data(std::vector<glm::vec2> dataIn, std::vector<glm::vec3> facesList, std::vector<glm::vec3> sourceFaceList, int targetSize) {
    std::vector<glm::vec2> correctedList;
    correctedList.resize(targetSize);
    int index = 0;
    for (auto fn : facesList) {
        int sourceIndex0 = sourceFaceList.at(index).x-1;
        int sourceIndex1 = sourceFaceList.at(index).y-1;
        correctedList[fn.x-1] = dataIn[sourceIndex0];
        correctedList[fn.y-1] = dataIn[sourceIndex1];
        correctedList[fn.z-1] = dataIn[sourceFaceList[index].z -1];
        index++;
    }
    return correctedList;
}

std::string getFileEnding(const std::string& fileName) {
    int len = fileName.size();
    std::string ending = "";
    for (int i = len; i > 0; i--) {
        if (fileName[i] == '.') {
            break;
        } else {
            ending += fileName[i];
        }
    }
    std::reverse(ending.begin(), ending.end());
    return ending;
}

std::unique_ptr<Mesh> loadMeshFromFile(const std::string &fileName) {
    auto ending = getFileEnding(fileName);
    if (ending.c_str() == std::string("gltf")) {
        return parseGLTF(parseJson(readFile(fileName)));
    }

    auto fileContents =  readFile(fileName);
    auto lines = splittolino(fileContents, "\n");
    auto positions = extractPositions(lines);
    auto normals = extractNormals(lines);
    auto uvs = extractUVs(lines);
    auto facesPos = extractFacesData(lines, 0);
    auto facesNormals = extractFacesData(lines, 2);
    auto facesUvs = extractFacesData(lines, 1);

//    //auto correcPos = correctV3Data(positions, facesPos);
//    auto corrNormals = correctV3Data(normals,  facesPos, facesNormals, positions.size());
//    auto corrUVs = correctV2Data(uvs, facesPos, facesUvs, positions.size());

    // Build the masterlists of all components
    std::vector<glm::vec3> posMasterList;
    std::vector<glm::vec2> uvMasterList;
    std::vector<glm::vec3> normalMasterList;
    for (auto f : facesPos) {
        // Iterate through the three parts forming the triangle face.
        for (int i = 0; i < 3; i++) {
            posMasterList.push_back(positions[f[i]-1]);
        }
    }
    for (auto f : facesNormals) {
        // Iterate through the three parts forming the triangle face.
        for (int i = 0; i < 3; i++) {
            normalMasterList.push_back(normals[f[i]-1]);
        }
    }
    for (auto f : facesUvs) {
        // Iterate through the three parts forming the triangle face.
        for (int i = 0; i < 3; i++) {
            uvMasterList.push_back(uvs[f[i]-1]);
        }
    }


    // Make it flat in memory for OpenGL
    auto posFlat = flattenV3(posMasterList);
    auto normFlat = flattenV3(normalMasterList);
    auto uvsFlat = flattenV2(uvMasterList);

    // Build the indices:
    // We just count from 0 up to the number of indices,
    // as we have already ordered all the vertex attributes
    // and amplified them if necessary.
    // Technically we would not even need an index buffer any more
    // as everything is completely described and in order by the
    // contents of the vertex buffers alone and we could just straight
    // out render from there.
    // As we are maintaining an indexed render pipeline, we will keep this in for now.
    std::vector<uint32_t> indicesFlat;
    for (int i = 0; i<facesPos.size() * 3; i++) {
        indicesFlat.push_back(i);
    }
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint vboPos;
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, posFlat.size() * 4, posFlat.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLuint vbUvs;
    glGenBuffers(1, &vbUvs);
    glBindBuffer(GL_ARRAY_BUFFER, vbUvs);
    glBufferData(GL_ARRAY_BUFFER, uvsFlat.size() * 4, uvsFlat.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    GLuint vboNormals;
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normFlat.size() * 4, normFlat.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, nullptr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(3, 1);

    GLuint vboIndices;
    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesFlat.size() * 4, indicesFlat.data(), GL_DYNAMIC_DRAW);

    auto mesh = std::make_unique<Mesh>();
    mesh->vao = vao;
    mesh->numberOfIndices = indicesFlat.size();
    mesh->instanceOffsetVBO = instanceVBO;
    return mesh;


}

void lightingOn() {
    glDefaultObjects->currentRenderState->lightingOn = true;
}

void lightingOff() {
    glDefaultObjects->currentRenderState->lightingOn = false;
}

void shadowOn() {
    glDefaultObjects->currentRenderState->shadows = true;
}

void shadowOff() {
    glDefaultObjects->currentRenderState->shadows = false;
}

void tilingOn(bool val) {
    glDefaultObjects->currentRenderState->tilingOn = val;
}

void tileData(int tileX, int tileY, int tileWidth, int tileHeight, int offsetX, int offsetY) {
    glDefaultObjects->currentRenderState->tileData = {tileX, tileY, tileWidth, tileHeight, offsetX, offsetY};
}


void flipUvs(bool val) {
    glDefaultObjects->currentRenderState->flipUvs = val;
}


glm::vec3 camFwd(Camera* cam) {
    auto fwd = glm::normalize((cam->lookAtTarget) - (cam->location));
    return fwd;
}

glm::vec3 cameraRightVector(Camera* camera) {
    auto fwd = glm::normalize((camera->lookAtTarget) - (camera->location));
    glm::vec3 right =glm::normalize(glm::cross(fwd, glm::vec3(0, 1, 0)));
    return glm::normalize(right);

}

glm::vec3 cameraUpVector(Camera* camera) {
    auto fwd = glm::normalize((camera->lookAtTarget) - (camera->location));
    auto right = cameraRightVector(camera);
    glm::vec3 up = glm::cross(right, fwd);
    return glm::normalize(up);
}

/**
 * Creates a ray object based on the camera and the current
 * mouse position on the screen.
 * The screen coordinates are un-projected to form a ray from the camera
 * into the world.
 * @param camera    The camera to create the ray for
 * @return          A ray from the camera into the world,
 *                  going through the mouse position on the near clip plane.
 */
Ray createRayFromCameraAndMouse(Camera* camera) {

    // Grab the mouse coordinates
    // and transform these into normalized-device coordinates (-1 to 1)
    float x = (2.0f * (float)mouse_x) / (float)window_width - 1.0f;
    float y = (2.0f * (float)mouse_y) / (float)window_height - 1.0f;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);

    // Create clip space by making them point into the world with negative z
    // and just a w=1.
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    // Unproject clip space into view-space by applying the inverse projection matrix.
    glm::vec4 ray_eye = glm::inverse(projectionMatrixForCamera(camera)) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    // Unproject into world matrix by applying the inverted view matrix (normally goes world to view, in this case
    // the other way around).
    glm::vec3 ray_wor = glm::inverse(viewMatrixForCamera(camera)) * ray_eye;
    ray_wor = normalize(ray_wor);

    // Now we have our direction into the world, and we can construct our actual ray.
    Ray ray;
    ray.origin = (camera->location);
    ray.direction = ray_wor;
    ray.maxLength = 100;

    // Special handling if this is an ortho camera.
    if (camera->type == CameraType::OrthoGameplay)
    {
        // For ortho projection things work a bit differently.
        // The origin is not the camera location, but we move in straight lines from the "plane" where the camera origin
        // exists, through the clip plane. There is no single point of origin here which leads to fanning out ray,
        // but this is a parallel projected ray instead.
        ray.origin = (camera->location) + cameraRightVector(camera) * ray_eye.x
                        + cameraUpVector(camera) * ray_eye.y;
        ray.direction = camFwd(camera);
    }

    return ray;
}



void collectJoints(JsonArray* armatureChildren, std::vector<Joint*>& targetVector, const JsonArray* nodes, Joint* parent) {
    for (auto c : armatureChildren->elements) {
        auto childIndex = c->value->floatValue;
        auto childNode = nodes->elements[childIndex]->value->objectValue;

        // Check if this node is the mesh.
        // If yes, skip it, we only collect joints.
        auto mesh = findByMemberName(childNode, "mesh");
        if (mesh) {
            continue;
        }
        auto name = findByMemberName(childNode, "name")->stringValue;
        Joint* joint = new Joint();
        joint->name = name;
        targetVector.push_back(joint);
        if (parent) {
            parent->children.push_back(joint);
        }
        auto translationArray = findByMemberName(childNode, "translation");
        if (translationArray && !translationArray->arrayValue->elements.empty()) {
            auto translationVertices = translationArray->arrayValue->elements;
            joint->translation = glm::vec3{translationVertices[0]->value->floatValue,
                                           translationVertices[1]->value->floatValue,
                                           translationVertices[2]->value->floatValue};
            joint->modelTranslation = glm::translate(glm::mat4(1), joint->translation);
        }

        auto rotationArray = findByMemberName(childNode, "rotation");
        if (rotationArray && !rotationArray->arrayValue->elements.empty()) {
            auto rotationValues = rotationArray->arrayValue->elements;
            joint->rotation = glm::quat(rotationValues[3]->value->floatValue,
                                        rotationValues[0]->value->floatValue,
                                        rotationValues[1]->value->floatValue,
                                        rotationValues[2]->value->floatValue);


            joint->modelRotation = glm::toMat4(joint->rotation);

        }

        joint->currentPoseLocalTransform = joint->modelTranslation * joint->modelRotation;
        joint->currentPoseGlobalTransform = parent ? (parent->currentPoseGlobalTransform * joint->currentPoseLocalTransform) : joint->currentPoseLocalTransform;

        auto children = findByMemberName(childNode, "children");
        if (children) {
            collectJoints(children->arrayValue, targetVector, nodes, joint);
        }
    }
}

/**
 * We only accept scenes with 1 mesh for now.
 * @param gltfJson the parsed json of the gltf
 * @return A Pointer to a newly created mesh object.
 */
std::unique_ptr<Mesh> parseGLTF(JsonElement* gltfJson) {

    // This is the embeddes binary data.
    // It is a base64 encoded string.
    auto dataBufferObj = queryJson(gltfJson, "/buffers[0]");
    auto dataUri = findByMemberName(dataBufferObj->value->objectValue, "uri")->stringValue;
    std::string uriDataString;
    std::stringstream ss(dataUri);
    // Use while loop to extract the last partial string after the delimiter.
    // In our case we expect only one part, the actual data.
    while (getline(ss, uriDataString, ',')) {}

    std:: string dec = base64::from_base64(uriDataString);
    std::vector<uint8_t> dataBinary;
    for (auto c: dec) {
        dataBinary.push_back(c);
    }


    // Armature data for skeletal mesh.
    // If we find a skin within the mesh object.
    // Otherwise we assume a static mesh.
    Skeleton* skeleton = nullptr;
    const auto nodesNode = queryJson(gltfJson, "/nodes");
    int skinIndex = -1;
    for (auto n : nodesNode->value->arrayValue->elements) {
        if (n->value->valueType == JsonValueType::Object) {
            auto potentialSkinMember = findByMemberName(n->value->objectValue, "skin");
            if (potentialSkinMember) {
                skinIndex = potentialSkinMember->intValue;
                break;
            }
        }
    }

    if (skinIndex > -1) {
        skeleton = new Skeleton();
        auto skinNode = queryJson(gltfJson, "/skins[" + std::to_string(skinIndex) + "]");
        int inverseBindMatricesIndex = (int) findByMemberName(skinNode->value->objectValue, "inverseBindMatrices")->floatValue;
        auto inverseBindAccessor = queryJson(gltfJson,"/accessors[" + std::to_string(inverseBindMatricesIndex) + "]");
        auto inverseBindViewIndex = (int) findByMemberName(inverseBindAccessor->value->objectValue, "bufferView")->floatValue;
        auto inverseBindBufferView = queryJson(gltfJson, "/bufferViews[" + std::to_string(inverseBindViewIndex) + "]");
        auto inverseBindBufferIndex = (int) findByMemberName(inverseBindBufferView->value->objectValue, "buffer")->floatValue;
        auto inverseBindBufferLen = findByMemberName(inverseBindBufferView->value->objectValue, "byteLength")->floatValue;
        auto inverseBindBufferByteOffset = (int) findByMemberName(inverseBindBufferView->value->objectValue, "byteOffset")->floatValue;

        auto invBindMatricesOffset = dataBinary.data() + inverseBindBufferByteOffset;
        int count = 1;
        // One matrix is 16 values, each 4 values form a column.
        std::vector<glm::mat4> invBindMatrices;
        std::vector<float> vals;
        std::vector<glm::vec4> vecs;
        for ( int i = 0; i < inverseBindBufferLen; i+=4) {
            auto val= (float*)(invBindMatricesOffset + i);
            printf("val: %f\n", *val);
            vals.push_back(*val);
            if (count % 4 == 0) {
                //printf("col------------\n");
                vecs.push_back({vals[0], vals[1], vals[2], vals[3]});
                vals.clear();
            }
            if (count % 16 == 0) {
                //printf("matrix ----------\n");
                invBindMatrices.push_back(glm::mat4(vecs[0], vecs[1], vecs[2], vecs[3]));
                vecs.clear();
            }
            count++;
        }
        // We search for a node called "Armature"
        for (auto n : nodesNode->value->arrayValue->elements) {
            auto obj = n->value->objectValue;
            auto name = findByMemberName(obj, "name")->stringValue;
            if (strContains(name, "Armature")) {
                auto children = findByMemberName(obj, "children")->arrayValue;
                collectJoints(children, skeleton->joints, nodesNode->value->arrayValue, nullptr);
            }
        }
        // Now we can associate the joints with the respective inverse bind matrix
        for (int i =0; i < skeleton->joints.size(); i++) {
            auto j = skeleton->joints[i];
            auto invBindMat = invBindMatrices[i];
            j->inverseBindMatrix = invBindMat;
        }

        // Now for the animations
        // Every animation has a number of references to so-called channel samplers.
        // Each sampler describes a specific joint and its translation, scale or rotation.
        // The sampler index points to the samplers array.
        // Each sampler maps a time point to an animation. (input -> output).
        // The indices within the sampler are accessor indices.
        // Reading out the animations:
        // For now just 1 animation.
        auto animationsNode = queryJson(gltfJson, "/animations");
        if (!animationsNode && !animationsNode->value->arrayValue->elements.empty()) {
            auto animNode = queryJson(gltfJson, "/animations[0]");
            auto samplersNode = findByMemberName(animNode->value->objectValue, "samplers");
            auto animName = findByMemberName(animNode->value->objectValue, "name");
            auto channelsNode = findByMemberName(animNode->value->objectValue, "channels");
            auto accessors = queryJson(gltfJson, "/accessors")->value->arrayValue->elements;
            auto bufferViews = queryJson(gltfJson, "/bufferViews")->value->arrayValue->elements;
            for (auto ch : channelsNode->arrayValue->elements) {
                int samplerIndex = (int) findByMemberName(ch->value->objectValue, "sampler")->floatValue;
                auto targetNode = findByMemberName(ch->value->objectValue, "target");
                auto targetPath = findByMemberName(targetNode->objectValue, "path")->stringValue;
                int targetJointIndex = (int) findByMemberName(targetNode->objectValue, "node")->floatValue;
                auto jointNode = nodesNode->value->arrayValue->elements[targetJointIndex];
                auto jointName = findByMemberName(jointNode->value->objectValue, "name")->stringValue;
                printf("joint channel:%s %s\n", jointName.c_str(), targetPath.c_str());
                auto samplerNode = samplersNode->arrayValue->elements[samplerIndex]->value->objectValue;
                auto samplerInput = (int)findByMemberName(samplerNode, "input")->floatValue;
                auto samplerOutput = (int) findByMemberName(samplerNode, "output")->floatValue;
                auto samplerInterpolation = findByMemberName(samplerNode, "interpolation")->stringValue;
                // Now lookup the accessors for input and output
                auto inputAccessor = accessors[samplerInput]->value->objectValue;
                auto outputAccessor = accessors[samplerOutput]->value->objectValue;
                auto inputCount = (int) findByMemberName(inputAccessor, "count")->floatValue;
                auto inputType = findByMemberName(inputAccessor, "type")->stringValue;
                auto outputType = findByMemberName(outputAccessor, "type")->stringValue;
                auto outputCount = (int) findByMemberName(outputAccessor, "count")->floatValue;
                auto inputBufferViewIndex = (int) findByMemberName(inputAccessor, "bufferView")->floatValue;
                auto outputBufferViewIndex = (int) findByMemberName(outputAccessor, "bufferView")->floatValue;
                auto inputBufferView = bufferViews[inputBufferViewIndex]->value->objectValue;
                auto outputBufferView = bufferViews[outputBufferViewIndex]->value->objectValue;
                auto inputBufferOffset = (int) findByMemberName(inputBufferView, "byteOffset")->floatValue;
                auto inputBufferLength = (int) findByMemberName(inputBufferView, "byteLength")->floatValue;
                auto outputBufferOffset = (int) findByMemberName(outputBufferView, "byteOffset")->floatValue;
                auto outputBufferLength = (int) findByMemberName(outputBufferView, "byteLength")->floatValue;
                auto inputDataPtr = dataBinary.data() + inputBufferOffset;
                std::vector<float> timeValues;
                for ( int i = 0; i < inputBufferLength; i+=4) {
                    auto val= (float*)(inputDataPtr + i);
                    timeValues.push_back(*val);
                }
                auto outputDataPtr = dataBinary.data() + outputBufferOffset;
                if (outputType == "VEC3") {
                    std::vector<glm::vec3> outputValues;
                    std::vector<float> fvals;
                    count = 1;
                    for ( int i = 0; i < outputBufferLength; i+=4) {
                        auto val= (float*)(outputDataPtr + i);
                        fvals.push_back(*val);
                        if (count % 3 == 0) {
                            outputValues.push_back(glm::vec3{fvals[0], fvals[1], fvals[2]});
                            fvals.clear();
                        }
                        count++;
                    }
                    for (int i=0; i<timeValues.size(); i++) {
                        printf("time val: %f -> %f/%f/%f\n", timeValues[i], outputValues[i].x, outputValues[i].y,
                               outputValues[i].z);
                    }

                } else if (outputType == "VEC4") {
                    std::vector<glm::vec4> outputValues;
                    std::vector<float> fvals;
                    count = 1;
                    for ( int i = 0; i < outputBufferLength; i+=4) {
                        auto val= (float*)(outputDataPtr + i);
                        fvals.push_back(*val);
                        if (count % 4 == 0) {
                            outputValues.push_back(glm::vec4{fvals[0], fvals[1], fvals[2], fvals[3]});

                            glm::quat quat = {fvals[3], fvals[0], fvals[1], fvals[2]};
                            glm::mat4 rotMat = glm::toMat4(quat);
                            printf("rotmat: %f/%f/%f/%f\n",
                                   glm::column(rotMat, 0).x,
                                   glm::column(rotMat, 1).x,
                                   glm::column(rotMat, 2).x,
                                   glm::column(rotMat, 3).x);
                            printf("rotmat: %f/%f/%f/%f\n",
                                   glm::column(rotMat, 0).y,
                                   glm::column(rotMat, 1).y,
                                   glm::column(rotMat, 2).y,
                                   glm::column(rotMat, 3).y);
                            printf("rotmat: %f/%f/%f/%f\n",
                                   glm::column(rotMat, 0).z,
                                   glm::column(rotMat, 1).z,
                                   glm::column(rotMat, 2).z,
                                   glm::column(rotMat, 3).z);
                            printf("rotmat: %f/%f/%f/%f\n",
                                   glm::column(rotMat, 0).w,
                                   glm::column(rotMat, 1).w,
                                   glm::column(rotMat, 2).w,
                                   glm::column(rotMat, 3).w);
                            fvals.clear();
                        }
                        count++;
                    }
                    for (int i=0; i<timeValues.size(); i++) {
                        printf("time val: %f -> %f/%f/%f/%f\n", timeValues[i], outputValues[i].x, outputValues[i].y,
                               outputValues[i].z, outputValues[i].w);
                    }
                } else if (outputType == "SCALAR") {
                    // TODO scalar
                }
            }
        }
    }

    // Mesh data
    auto primitivesNode = queryJson(gltfJson, "/meshes[0]/primitives[0]");
    auto attrObj = findByMemberName(primitivesNode->value->objectValue, "attributes");
    int posIndex = (int) findByMemberName(attrObj->objectValue, "POSITION")->floatValue;
    auto uvIndex = (int) findByMemberName(attrObj->objectValue, "TEXCOORD_0")->floatValue;
    auto normalIndex =(int) findByMemberName(attrObj->objectValue, "NORMAL")->floatValue;
    auto indicesIndex = (int) findByMemberName(primitivesNode->value->objectValue, "indices")->floatValue;

    auto posAccessor = queryJson(gltfJson,"/accessors[" + std::to_string(posIndex) + "]");
    auto uvAccessor = queryJson(gltfJson,"/accessors[" + std::to_string(uvIndex) + "]");
    auto normalAccessor = queryJson(gltfJson,"/accessors[" + std::to_string(normalIndex) + "]");
    auto indicesAccessor = queryJson(gltfJson,"/accessors[" + std::to_string(indicesIndex) + "]");

    auto posBufferViewIndex = (int) findByMemberName(posAccessor->value->objectValue, "bufferView")->floatValue;
    auto posBufferView = queryJson(gltfJson, "/bufferViews[" + std::to_string(posBufferViewIndex) + "]");
    auto posBufferIndex = (int) findByMemberName(posBufferView->value->objectValue, "buffer")->floatValue;
    auto posBufferLen = findByMemberName(posBufferView->value->objectValue, "byteLength")->floatValue;
    auto posBufferByteOffset = (int) findByMemberName(posBufferView->value->objectValue, "byteOffset")->floatValue;
    auto posGlTargetType = (int) findByMemberName(posBufferView->value->objectValue, "target")->floatValue;

    auto uvBufferViewIndex = (int) findByMemberName(uvAccessor->value->objectValue, "bufferView")->floatValue;
    auto uvBufferView = queryJson(gltfJson, "/bufferViews[" + std::to_string(uvBufferViewIndex) + "]");
    auto uvBufferIndex = (int) findByMemberName(uvBufferView->value->objectValue, "buffer")->floatValue;
    auto uvBufferLen = findByMemberName(uvBufferView->value->objectValue, "byteLength")->floatValue;
    auto uvBufferByteOffset = (int) findByMemberName(uvBufferView->value->objectValue, "byteOffset")->floatValue;
    auto uvGlTargetType = (int) findByMemberName(uvBufferView->value->objectValue, "target")->floatValue;

    auto normalBufferViewIndex = (int) findByMemberName(normalAccessor->value->objectValue, "bufferView")->floatValue;
    auto normalBufferView = queryJson(gltfJson, "/bufferViews[" + std::to_string(normalBufferViewIndex) + "]");
    auto normalBufferIndex = (int) findByMemberName(normalBufferView->value->objectValue, "buffer")->floatValue;
    auto normalBufferLen = findByMemberName(normalBufferView->value->objectValue, "byteLength")->floatValue;
    auto normalBufferByteOffset = (int) findByMemberName(normalBufferView->value->objectValue, "byteOffset")->floatValue;
    auto normalGlTargetType = (int) findByMemberName(normalBufferView->value->objectValue, "target")->floatValue;

    auto indicesBufferViewIndex = (int) findByMemberName(indicesAccessor->value->objectValue, "bufferView")->floatValue;
    auto indicesBufferView = queryJson(gltfJson, "/bufferViews[" + std::to_string(indicesBufferViewIndex) + "]");
    auto indicesBufferIndex = (int) findByMemberName(indicesBufferView->value->objectValue, "buffer")->floatValue;
    auto indicesBufferLen = findByMemberName(indicesBufferView->value->objectValue, "byteLength")->floatValue;
    auto indicesBufferByteOffset = (int) findByMemberName(indicesBufferView->value->objectValue, "byteOffset")->floatValue;
    auto indicesGlTargetType = (int) findByMemberName(indicesBufferView->value->objectValue, "target")->floatValue;
    auto indexCount = (int) findByMemberName(indicesAccessor->value->objectValue, "count")->floatValue;
    auto indexComponentType = (GLenum) findByMemberName(indicesAccessor->value->objectValue, "componentType")->floatValue;



    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint posBuffer;
    glGenBuffers(1, &posBuffer);
    glBindBuffer(posGlTargetType, posBuffer);
    glBufferData(posGlTargetType, posBufferLen, dataBinary.data() + posBufferByteOffset, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(uvGlTargetType, uvBuffer);
    glBufferData(uvGlTargetType, uvBufferLen, dataBinary.data() + uvBufferByteOffset , GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(normalGlTargetType, normalBuffer);
    glBufferData(normalGlTargetType, normalBufferLen, dataBinary.data() + normalBufferByteOffset, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    std::vector<float> instanceOffsets = {0, 0};
    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 1, instanceOffsets.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glVertexAttribDivisor(3, 1);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(indicesGlTargetType, indexBuffer);
    glBufferData(indicesGlTargetType, indicesBufferLen, dataBinary.data() + indicesBufferByteOffset, GL_STATIC_DRAW);

    auto mesh = std::make_unique<Mesh>();
    mesh->skeleton = skeleton;
    mesh->vao = vao;
    mesh->instanceOffsetVBO = instanceVBO;
    mesh->numberOfIndices = indexCount;
    mesh->indexDataType = indexComponentType;
    glBindVertexArray(0);

    return mesh;


}

void uvScale(float val) {
    glDefaultObjects->currentRenderState->uvScale = val;
}

void forceShader(Shader *shader) {
    glDefaultObjects->currentRenderState->forcedShader = shader;
}

void setUniformFloat(int location, float val, Shader *shader) {
    if (shader) {
        bindShader(shader);
    }
    glUniform1f(location, val);

}

void deferredStart() {
    glDefaultObjects->currentRenderState->deferred = true;

}

void deferredEnd() {
    glDefaultObjects->currentRenderState->deferred = false;
    // Actually execute the collected draw calls
    // First do the shadow pass, draw everything ino the shadow map
    // Temp: skip shadow pass for now
    for (auto dc : glDefaultObjects->currentRenderState->drawCalls){
        dc.shadowPass =true;
        if (dc.mesh) {
            drawMeshCallExecution(dc);
        } else {
            drawPlaneCallExecution(dc);
        }

    }


    // Next the actual draw calls
    for (auto dc : glDefaultObjects->currentRenderState->drawCalls){
        dc.shadowPass =false;
        if (dc.mesh) {
            drawMeshCallExecution(dc);
        } else {
            drawPlaneCallExecution(dc);
        }

    }


    glDefaultObjects->currentRenderState->drawCalls.clear();
}

std::unique_ptr<Texture> createEmptyTexture(int w, int h) {
    auto pixels = (uint8_t *) _aligned_malloc(w*h*4, 32);
    auto bm = new Bitmap();
    bm->width = w;
    bm->height = h;
    bm->pixels = pixels;
    return createTextureFromBitmap(bm);
}

void textScale(glm::vec2 val) {
    glDefaultObjects->currentRenderState->textScale = val;
}

void font(Bitmap *fontBitmap) {
    glDefaultObjects->currentRenderState->font = fontBitmap;
}

void setInstanceColors(std::vector<glm::vec4> colors) {
    glDefaultObjects->currentRenderState->instanceColors.clear();
    glDefaultObjects->currentRenderState->instanceColors = colors;
}

void setInstanceTints(std::vector<glm::vec4> tints) {
    glDefaultObjects->currentRenderState->instanceTints.clear();
    glDefaultObjects->currentRenderState->instanceTints = tints;
}

void setInstanceMatrices(std::vector<glm::mat4> matrices) {
    glDefaultObjects->currentRenderState->instanceMatrices.clear();
    glDefaultObjects->currentRenderState->instanceMatrices = matrices;
}

void instanceOffsets(std::vector<glm::vec2> offsets) {
    glDefaultObjects->currentRenderState->instanceOffsets.clear();
    glDefaultObjects->currentRenderState->instanceOffsets = offsets;
}

int random(int min, int max) {

    return  min + (rand() % (max - min + 1));
}

void resetParticleEffect(ParticleEffect* pe) {
    pe->done = false;
    pe->initialized = false;
    pe->finishedParticles = 0;
}


void updateParticleEffect(ParticleEffect *pe) {
    if (pe->done) {
        return;
    }

    auto ftSecs = ftMicros / 1000.0f / 1000.0f;
    pe->lifeTimeInSeconds += ftSecs;
    pe->lastSpawnTime += ftSecs;

    if (!pe->initialized ) {
        for (int i = 0; i < pe->number; i++) {
            Particle* p = new Particle();
            p->location = pe->location;
            p->direction = glm::normalize(glm::vec3{random(-1, 1), random(-1, 1), random(-1, 1)});
            p->accel = p->direction * 34.0f * pe->initialSpeed;
            p->scale = glm::vec3{random(2, 3)/10.0f, random(2,3)/10.0f, random(2, 3)/10.0f};
            pe->particles.push_back(p);
        }

        pe->initialized = true;
        pe->lastSpawnTime = 0;



    }

    // Now handle existing particles
    for (auto p : pe->particles) {
        p->lifeTime += ftSecs;
        if (p->lifeTime < pe->maxLifeInSeconds && p->location.y >= 0) {
            p->accel += glm::vec3(0, pe->gravityValue * 5.0f * ftSecs, 0);
            p->location += p->accel * 0.55f  * ftSecs;
        } else {
            if (pe->oneShot) {
                if (!p->remove) {
                    p->remove = true;
                    pe->finishedParticles++;
                }
            } else {
                p->location = pe->location;
                p->direction = glm::normalize(glm::vec3{random(-1, 1), random(-1, 1), random(-1, 1)}) ;
                p->accel = p->direction * 34.0f * pe->initialSpeed;
                p->scale = glm::vec3{random(2, 6) / 19.0f , random(2, 5)/19.0f, random(2,5)/19.0f};
                p->lifeTime = 0;
            }
        }
    }

    if (pe->finishedParticles == pe->number) {
        pe->done = true;
        for (auto p : pe->particles) {
            delete(p);
        }
        pe->particles.clear();
    }
}

void drawParticleEffect(bool instancedRender) {
    ParticleEffect* pe  = glDefaultObjects->currentRenderState->particleEffect;
    bindMesh(pe->mesh);
    bindTexture(pe->texture);

    if (!instancedRender) {
        for (auto p : pe->particles) {
            location(p->location);
            rotation(p->rotation);
            auto livedPercent = p->lifeTime / pe->maxLifeInSeconds;
            scale({p->scale.x * (1- livedPercent) , p->scale.y * (1 - livedPercent), p->scale.z * (1- livedPercent)});
            overrideAlpha(1 - livedPercent);
            drawMesh();

        }
    } else {
        deferredStart();
        std::vector<glm::mat4> matrices;
        for (auto p : pe->particles) {
            // location(p->location);
            // rotation(p->rotation);
            auto livedPercent = p->lifeTime / pe->maxLifeInSeconds;
            //scale({p->scale.x * (1- livedPercent) , p->scale.y * (1 - livedPercent), p->scale.z * (1- livedPercent)});
            overrideAlpha(1 - livedPercent);
            drawMesh();

            glm::mat4 mattrans = glm::translate(glm::mat4(1), p->location);
            glm::mat4 matscale = glm::scale(glm::mat4(1), {p->scale.x * (1- livedPercent) ,
                                                                        p->scale.y * (1 - livedPercent),
                                                                        p->scale.z * (1- livedPercent)});
            glm::mat4 matworld = mattrans * matscale;
            matrices.push_back(matworld);
        }

        setInstanceMatrices(matrices);
        drawMeshInstanced(matrices.size());
        deferredEnd();

    }

    // Instanced rendering


    overrideAlpha(1);
}

void bindParticleEffect(ParticleEffect *pe) {
    glDefaultObjects->currentRenderState->particleEffect = pe;
}

void overrideAlpha(float val) {
    glDefaultObjects->currentRenderState->overrideAlpha = val;
}

void tint(glm::vec4 col) {
    glDefaultObjects->currentRenderState->tint = col;
}


void panUVS(glm::vec2 pan) {
    glDefaultObjects->currentRenderState->panUVS = pan;
}

void bitmapBlitModeOn() {
    glDefaultObjects->currentRenderState->inBitMapBlitMode = true;
}

void bitmapBlitModeOff() {
    glDefaultObjects->currentRenderState->inBitMapBlitMode = false;
}



void updateTexture(int w, int h, Texture* texture) {
    updateTextTexture(w, h, texture);
}






void Raytracer::render(int pixelWidth, int pixelHeight) {
    // First, generate the rays.
    // For now, only one ray goes out, no bounces.
    // The cameras go out from the camera position through each
    // virtual pixel.
    // Default camera position is at the origin, looking down the negative z-axis.
    // Focal length = 1
    // FOV = 90 ( So half-angle is 45 degrees)
    // So the pixel is the vector addition of moving forward and then moving 45 degrees to the left and then the
    // distance per pixel to the right.
    glm::vec3 leftTopPoint = glm::vec3(0, 0, -1) + glm::vec3(-1, 0, 0);
    float pixelWidthSpacing = 2.0f / pixelWidth;
    float pixelHeightSpacing = 1.8f / pixelHeight;
    glm::vec3 camPosition = {0, 0, 0};
    for (int x = 0; x < pixelWidth; x++) {
        for (int y = 0; y < pixelHeight; y++) {
            glm::vec3 targetPoint = leftTopPoint + glm::vec3(pixelWidthSpacing * x, pixelHeightSpacing * -y, 0);
            glm::vec3 dir = glm::normalize(targetPoint - camPosition);
            printf("dir: %f/%f/%f\n", dir.x, dir.y, dir.z);

            // Now we have a ray, and we shoot it into the world to see if it collides with geometry.
            // First without any acceleration structures, this will come later.
            for (auto m : _meshes) {

            }
        }
    }

}




// Sets up the internal frustum based VAO
// and also the shader for debug rendering.

glm::vec2 modelToScreenSpace(glm::vec3 model, glm::mat4 matWorld, Camera* camera) {

    glm::vec4 clipSpace = projectionMatrixForCamera(camera) * viewMatrixForCamera(camera) * matWorld * glm::vec4(model, 1);
    glm::vec4 ndc = clipSpace / clipSpace.w;
    glm::vec2 ss = {((ndc.x+1)/2) * window_width, ((ndc.y+1)/2)*window_height};
    return ss;
}


void activateFrameBuffer(const FrameBuffer* fb) {
    if (fb) {
        glBindFramebuffer(GL_FRAMEBUFFER, fb->handle);
   } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}

void depthTestOn() {
    glDefaultObjects->currentRenderState->depthTest = true;

}

void depthTestOff() {
    glDefaultObjects->currentRenderState->depthTest = false;

}

void useWorldMatrix(bool value) {
    glDefaultObjects->currentRenderState->useWorldMatrix = value;
}

void setWorldMatrix(glm::mat4 worldMatrix) {
    glDefaultObjects->matWorld = worldMatrix;
}

void setSkinnedDraw(bool value) {
    glDefaultObjects->currentRenderState->skinnedDraw = value;
}

void setBoneMatrices(std::vector<glm::mat4> boneMatrices) {
    glDefaultObjects->boneMatrices = boneMatrices;
}




void bindShader(Shader* shader) {
    glDefaultObjects->currentRenderState->shader = shader;
    glUseProgram(shader->handle);
}













FBButton::FBButton(const std::string &text, glm::vec3 location, glm::vec2 size, FBFont *font) : font(font), text(text), _location(location), size(size){


}

void FBButton::render() {

    bindTexture(nullptr);
    if (state == EButtonState::HOVERED) {
        scale({size.x * 0.9, size.y * 0.9, 1});
    } else {
        scale({size.x, size.y, 1});
    }

    location(_location);
    foregroundColor({0.4, 0.4, 0.8, 1});
    drawPlane();

    scale({1, 1,1 });
    foregroundColor({1, 1,1, 1});
    font->renderText(text, {_location.x  + 246, _location.y - size.y/4 , _location.z  + 0.1});


}

bool FBButton::mouseOver() {
    if (mouse_x > (_location.x - size.x/2) && mouse_x < (_location.x + size.x/2) &&
        mouse_y > (_location.y - size.y/2) && mouse_y < (_location.y + size.y/2)
    ) {
        state = EButtonState::HOVERED;
        if (lbuttonUp) {
            state = EButtonState::CLICKED;
        }
        return true;
    }

    state = EButtonState::NONE;
    return false;
}
#endif