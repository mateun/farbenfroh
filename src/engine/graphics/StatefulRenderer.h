//
// Created by mgrus on 26.03.2025.
//

#ifndef STATEFULRENDERER_H
#define STATEFULRENDERER_H

#include <GL\glew.h>
#include <string>
#include <glm\glm.hpp>
#include <vector>
#include <memory>

#include <engine/graphics/Texture.h>
#include <engine/graphics/PlanePivot.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/Shader.h>
#include <engine/io/json.h>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/FrameBuffer.h>

class Light;
class MeshDrawData;


struct Result {
    bool ok;
    std::vector<std::string> errors;
};

struct MonitorResolution {
    uint64_t width;
    uint64_t height;
    uint64_t frequency;
    uint64_t bitsPerPixel;

};

struct Particle {
    glm::vec3 accel;
    glm::vec3 location;
    glm::vec3 scale;
    glm::vec3 rotation;
    glm::vec3 direction = {0, 1, 0};
    glm::vec4 color = {1, 1, 1, 1};
    float lifeTime = 0;
    bool remove = false;

};

struct ParticleEffect {
    bool initialized = false;
    glm::vec3 location = {0, 0, 0};
    glm::vec3 rotation = {0, 0, 0};
    int number = 1;
    int finishedParticles = 0;
    float scaleMin = 0;
    float scaleMax = 5;
    float maxLifeInSeconds = 2;
    float lifeTimeInSeconds = 0;
    float spawnLag = 0.1;
    float lastSpawnTime = 0;
    float spawnRatePerSecond = 10;
    bool useGravity = true;
    bool oneShot = true;
    bool done = false;
    float gravityValue = -9.81f;
    float initialSpeed = 1;
    Mesh* mesh = nullptr;
    Texture* texture =nullptr;
    std::vector<Particle*> particles;


};

enum class ColorFormat {
    RGBA,
    BGRA,
    RGB,
    R,
    B,
    G
};

struct GridData {
    float scale = 1;
    glm::vec4 color = {0.6, 0.8, 0.2, .5};
    glm::vec3 loc = {0, 0, 0};
    int numLines = 100;
    GLuint vao;
    Camera* camera = nullptr;
};


struct DrawCall {
    glm::vec3 location;
    glm::vec3 scale;
    glm::vec3 rotation;
    GLfloat uvScale;
    glm::vec4 foregroundColor;
    float overrideAlpha = 1.0f;
    Mesh* mesh = nullptr;
    Texture *texture;

    bool lightingOn;
    bool flipUvs;
    bool tilingOn;
    glm::vec3 lightDirection{-0.6, -10, -0.5};

    bool shadows;
    Camera *camera = nullptr;
    Camera *shadowMapCamera = nullptr;

    bool shadowPass = false;
    int instances = 1;
    std::vector<glm::vec2> instanceOffsets;
    std::vector<glm::mat4> instanceMatrices;
    std::vector<glm::vec4> instanceColors;
    std::vector<glm::vec4> instanceTints;

};



struct TileData {
    int tileX;
    int tileY;
    int tileWidth;
    int tileHeight;
    int tileOffsetX;
    int tileOffsetY;
};


/**
 * This holds the state for render commands
 * which is valid between a beginBatch and endBatch call.
 * The user may issue several of these, each time a
 * new state is created.
 */
struct RenderState {
    glm::vec4 foregroundColor;
    glm::vec3 location;
    glm::vec3 rot;
    glm::mat4 *rotMatrix = nullptr;
    glm::vec3 scale;
    glm::vec2 textScale = {1, 1};
    glm::vec2 panUVS = {0, 0};
    glm::vec4 clearColor= {0, 0, 0, 1};
    float uvScale = 1;
    float overrideAlpha = 1.0f;
    std::vector<glm::vec2> instanceOffsets;
    std::vector<glm::mat4> instanceMatrices;
    std::vector<glm::vec4> instanceColors;
    std::vector<glm::vec4> instanceTints;
    Shader* shader = nullptr;
    Shader* forcedShader = nullptr;
    Texture* texture = nullptr;
    Texture* normalMap = nullptr;
    int normalMapTextureUnit = 2;
    Texture* skyboxTexture = nullptr;
    Bitmap *font = nullptr;
    Mesh* mesh = nullptr;
    ParticleEffect *particleEffect = nullptr;
    Camera* camera = nullptr;
    Camera* shadowMapCamera = nullptr;
    Bitmap* renderTargetBitmap = nullptr;
    bool lightingOn = true;
    bool tilingOn;
    bool flipUvs;
    bool shadows = false;
    bool deferred = false;
    bool depthTest = true;
    bool inBitMapBlitMode = false;
    bool useWorldMatrix = false;

    bool skinnedDraw = false;
    bool textDraw = false;

    Light* currentLight = nullptr;


    TileData tileData;

    std::vector<DrawCall> drawCalls;
    glm::vec4 tint = {1, 1, 1, 1};
    const FrameBuffer* renderToFrameBuffer = nullptr;
};



struct GLDefaultObjects {
    Shader* shadowMapShader = nullptr;
    Shader* singleColorShader = nullptr;
    Shader* singleColorShaderInstanced = nullptr;
    Shader* skinnedShader = nullptr;
    Shader* skinnedShadowMapShader = nullptr;
    Shader* texturedSkinnedShader = nullptr;
    Shader* instancedShader = nullptr;
    Shader* texturedShaderLit = nullptr;
    Shader* texturedShaderUnlit = nullptr;
    Shader* skyboxShader = nullptr;
    Shader* gridShader = nullptr;
    Shader* gridPostProcessShader = nullptr;
    Shader* spriteShader = nullptr;
    GLuint quadVAO = -1;
    GLuint textQuadVAO = -1;
    GLuint* gridVAO = nullptr;
    GLuint skyboxVAO;
    GLuint shadowMapFramebuffer = 0;
    GLuint quadPosBuffer = 0;
    GLuint quadIndexBuffer = 0;
    GLuint quadUVBuffer = 0;
    std::unique_ptr<Texture> shadowMap = nullptr;
    RenderState* currentRenderState = nullptr;
    std::unique_ptr<FrameBuffer> gridFBO = nullptr;
    Camera* defaultUICamera = nullptr;
    bool inBatchMode = false;
    std::vector<glm::mat4> boneMatrices;

    // Batch lists
    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;

    glm::mat4 matWorld;

    // For skinned rendering
    GLuint boneIndexBuffer;
    GLuint boneWeightBuffer;

};

/**
* This is the compatibility layer for the
* legacy "graphics.h" c style render functions:
*/
class StatefulRenderer {
public:

    static GridData* createGrid(int lines = 100);
    static GLuint createGridVAO(int lines = 100);
    static void drawText(const char* text, int x, int y, Bitmap* font);
    static void drawBitmap(int x, int y, Bitmap* bm);
    static void drawBitmapTile(int posx, int posy, int tileSize, int tilex, int tiley, Bitmap* bitmap, int offsetX  = 0, int offsetY = 0);
    static std::unique_ptr<Bitmap> loadBitmap(const char* fileName);
    static void setPixel(int x, int y, int r, int g, int b, int a);

    void prepareShadowMapTransformationMatrices();
    void prepareTransformationMatrices(glm::vec3 location, glm::vec3 scale, glm::vec3 rotation, Camera* camera, Camera* shadowMapCamera, bool shadows);

    void drawPlaneUnlit();

    void drawPlaneLit(Light *directionalLight, const std::vector<Light *> &pointLights);

    static void drawPlane();
    static void drawPlane(Light* directionalLight, const std::vector<Light*>& pointLights);
    static void drawMesh();
    static void drawMeshIntoShadowMap(MeshDrawData dd, Light* l);
    static void drawSkybox();
    static void drawMeshSimple();
    static void drawMeshInstanced(int num);
    static void drawParticleEffect(bool instancedRender = false);
    static void drawGrid(GridData* gridData = {}, glm::ivec2 viewPortDimensions = glm::ivec2{getApplication()->scaled_width(), getApplication()->scaled_height()}, bool blurred = false);

    DrawCall createPlaneDrawCall();

    void drawPlaneCallExecution(DrawCall dc);

    static void drawBitmap(int x, int y, uint8_t* bitmapPixels);
    static glm::mat4 getWorldMatrixFromGlobalState();
    static std::unique_ptr<Texture> createTextureFromFile(const std::string& fileName);
    static std::unique_ptr<Texture> createTextureFromBitmap(Bitmap* bm, ColorFormat colorFormat = ColorFormat::RGBA);
    static std::unique_ptr<Texture> createCubeMapTextureFromDirectory(const std::string &dirName, ColorFormat colorFormat, const std::string& fileType = "png");
    static std::unique_ptr<FrameBuffer> createShadowMapFramebufferObject(glm::vec2 size);
    static void beginBatch();
    static void endBatch();
    static void foregroundColor(glm::vec4 col);

    // This can be used with textures, will multiply with the texel color
    static void tint(glm::vec4 col);

    static void scale(glm::vec3 val);
    static void textScale(glm::vec2 val);
    static void panUVS(glm::vec2 pan);
    static void location(glm::vec3 loc);
    static void rotation(glm::vec3 rot);
    static void overrideAlpha(float val);
    static void instanceOffsets(std::vector<glm::vec2> offsets);
    static void bindTexture(Texture* tex);
    static void bindNormalMap(Texture* tex, int unit = 2);
    static void bindSkyboxTexture(Texture* tex);
    static void unbindSkyboxTexture();
    static void tilingOn(bool val);
    static void tileData(int tileX, int tileY, int tileWidth, int tileHeight, int offsetX =0, int offsetY = 0);
    static void bindMesh(Mesh* mesh);
    static void bindParticleEffect(ParticleEffect* pe);
    static void renderTargetBitmap(Bitmap* bm);
    static void bindCamera(Camera* camera);
    static void bindShadowMapCamera(Camera* camera);
    static void font(Bitmap* fontBitmap);
    static std::vector<std::string> getAllActiveMonitors();
    static std::vector<MonitorResolution> getMonitorResolutions(const std::string& deviceName);
    static bool changeResolution(int width, int height, int refreshRate, const std::string& deviceName, bool goFullscreen = false);
    static void exitFullscreen();
    static void mouseVisible(bool val);
    static void lightingOn();
    static void lightingOff();
    static void shadowOff();
    static void shadowOn();

    [[deprecated]]
    static void wireframeOn(float lineWidth = 2.0f);
    static void wireframeOff();
    static void deferredStart();
    static void deferredEnd();
    static void flipUvs(bool val);
    static void uvScale(float val);
    static void forceShader(Shader* shader);
    static void setUniformFloat(int location, float val, Shader* shader = nullptr);
    static void updateParticleEffect(ParticleEffect* pe);
    static void resetParticleEffect(ParticleEffect* pe);
    static void bitmapBlitModeOn();
    static void bitmapBlitModeOff();
    static void depthTestOn();
    static void depthTestOff();
    static void useWorldMatrix(bool value);
    static void setWorldMatrix(glm::mat4 worldMatrix);
    static void setSkinnedDraw(bool value);
    static void setBoneMatrices(std::vector<glm::mat4> boneMatrices);
    static glm::mat4 calculateWorldTransform(Joint* j, glm::mat4 currentTransform);
    static GLuint createQuadVAO(PlanePivot pivot = PlanePivot::center);

    static glm::vec2 modelToScreenSpace(glm::vec3 model, glm::mat4 matWorld, Camera* camera);

    static std::unique_ptr<Texture> createEmptyTexture(int w, int h);
    static std::unique_ptr<Texture> createEmptyFloatTexture(int w, int h);
    static std::unique_ptr<Texture> createTextTexture(int w, int h);
    static void updateTexture(int w, int h, Texture* texture);
    static std::unique_ptr<Mesh> loadMeshFromFile(const std::string& fileName);
    static Result createShader(const std::string& vsrc, const std::string& fsrc, Shader* target);
    static Result createComputeShader(const std::string& source, Shader* target);
    static std::unique_ptr<Mesh> parseGLTF(JsonElement* gltfJson);
    static void prepareTransformationMatrices(glm::mat4 matworld, Camera* camera, Camera* shadowMapCamera, bool shadows);

    void prepareTransformationMatrices(DrawCall dc);

    void prepareShadowMapTransformationMatrices(glm::vec3 location, glm::vec3 scale, glm::vec3 rotation, Camera* shadowMapCamera);

    void prepareShadowMapTransformationMatrices(DrawCall &dc);

    static void updateAndDrawText(const char *text, Texture *pTexture, int screenPosX, int screenPosY, int screenPosZ = -0.7);

    // Creates a color framebuffer incl. depthbuffer
    static std::unique_ptr<FrameBuffer> createFrameBuffer(int width, int height, bool hdr = false, bool additionalColorBuffer = false);

    // Creates a color framebuffer based on the given texture and a default depthtexture.
    // An additional color texture can be used to create a second color attachment, e.g. for bloom effects.
    static std::unique_ptr<FrameBuffer> createFrameBufferWithTexture(int width, int height, std::shared_ptr<Texture> colorTexture, std::shared_ptr<Texture> colorTexture2 = nullptr);

    static bool keyPressed(int key);

    static void activateFrameBuffer(const FrameBuffer* fb);

private:
    inline static GLDefaultObjects* glDefaultObjects = new GLDefaultObjects();

};


#endif //STATEFULRENDERER_H
