#pragma once

#include <functional>
#include <Windows.h>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <engine/fx/ParticleSystem.h>
#include <GL/glew.h>
#include <engine/input/UpdateSwitcher.h>
#include <engine/lighting/Light.h>
#include <glm/glm.hpp>
#include "glm/detail/type_quat.hpp"
#include "assimp/scene.h"
#include "../src/engine/game/game_model.h"
#include <engine/graphics/PlanePivot.h>
#include <engine/animation/skeleton.h>
#include <engine/graphics/Bitmap.h>

class FBFont;
class Camera;

namespace ozz::animation {
    class Skeleton;
}

extern int scaled_width, scaled_height;
class JsonElement;

struct BITMAP_FILE {
	BITMAPFILEHEADER bmfileHeader;
	BITMAPINFOHEADER bminfoHeader;

};


struct Result {
    bool ok;
    std::vector<std::string> errors;
};











class PostProcessEffect {
public:
    PostProcessEffect();
    virtual const FrameBuffer* apply(const FrameBuffer* sourceFrameBuffer, const Camera* camera) = 0;

protected:
    std::unique_ptr<Mesh> quadMesh;
    std::unique_ptr<FrameBuffer> effectFrameBuffer;
};

class GammaCorrectionEffect : public PostProcessEffect {
public:
    GammaCorrectionEffect();
    const FrameBuffer* apply(const FrameBuffer* sourceFrameBuffer, const Camera* camera) override;

private:
    Shader* gammaCorrectionShader = nullptr;

};

class BloomEffect : public PostProcessEffect {
public:
    BloomEffect();
    const FrameBuffer* apply(const FrameBuffer *sourceFrameBuffer, const Camera* camera) override;

private:
    Shader* bloomShader = nullptr;
    Shader * quadShader = nullptr;
    Shader * gaussShader = nullptr;
    std::unique_ptr<FrameBuffer> bloomFBO = nullptr;
    std::vector<std::unique_ptr<FrameBuffer>> blurFBOs;
};

/**
 * This class gets a list of objects which it may not collide with.
 * E.g. a planet, wals, etc.
 * The component takes care if any collision took place and can be asked by the camera
 * to avoid further movement into the colliding object.
 */
class CameraCollider {

public:
    struct CollisionCandidate {
        glm::vec3 location;
        float radius;
    };

    bool collides(glm::vec3 cameraPosition);

    CameraCollider(std::vector<CollisionCandidate*>);

private:
    std::vector<CollisionCandidate*> collisionCandidates;


};



struct TileData {
    int tileX;
    int tileY;
    int tileWidth;
    int tileHeight;
    int tileOffsetX;
    int tileOffsetY;
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

/**
 * An FBButton consists of a text and a size and location.
 * You can ask the button if the mouse is over it.
 */
class FBButton {
public:

    enum class EButtonState {
        NONE,
        HOVERED,
        CLICKED

    };

    FBButton(const std::string& text, glm::vec3 location, glm::vec2 size, FBFont* font);
    void render();
    bool mouseOver();

    glm::vec2 size;
    glm::vec3 _location;
    std::string text;
    FBFont *font;
    EButtonState state = EButtonState::NONE;
};

enum class ColorFormat {
    RGBA,
    BGRA,
    RGB,
    R,
    B,
    G
};

namespace gru {

    struct UVs {
        float left;
        float top;
        float right;
        float bottom;
    };

    // A tile in a tileMap in pixel coordinates.
    struct TextureTile {

        TextureTile(glm::vec2 tl, glm::vec2 br, glm::vec2 atl) {
            topLeft = tl;
            bottomRight = br;
            atlas = atl;
        }

        glm::vec2 topLeft;
        glm::vec2 bottomRight;
        glm::vec2 atlas;

        UVs getTextureCoords() {
            auto tileWidth = bottomRight.x - topLeft.x;
            auto tileHeight = bottomRight.y - topLeft.y;
            float uvWidth = tileWidth / atlas.x;
            float uvHeight = tileHeight / atlas.y;
            float uvLeft = topLeft.x / atlas.x;
            float uvRight = uvLeft + uvWidth;
            float uvTop = 1 -  (topLeft.y / atlas.y);
            float uvBottom = uvTop - uvHeight;

            return { uvLeft, uvTop, uvRight, uvBottom};
        }
    };




    /***
     * This class allows for efficient rendering of many sprites in one go.
     * The idea is that we shall not issue a draw call per sprite.
     * Instead we collect all sprites to be rendered here.
     * Conveniently, the user can give us the sprites in pixel coordinates, which is most natural
     * for the game developer.
     * Internally we transform these coordinates into clipspace coordinates and store them in a list.
     * When the user calls "render", we make one update of the VBO containing every sprite.
     *
     */
    class SpriteBatch {

    public:

        SpriteBatch(int numberOfSpritesReservered);

        // Add a "sprite" by giving its position and size in screen space, i.e.
        // in pixels.
        // For example, if the screen is 640 x 480 and we give a sprite at position
        // 320/240, it will end up in clip space coordinates 0,0.
        // ClipSpace is -1 to 1 in all dimensions.
        // The formula is
        // clipSpace is: 2 * (pos/screenDimension) -1.
        // 2 * (320/640) -1 = 2 * 0.5 - 1 = 1 - 1 = 0
        // We use as a a depth indication
        int addSprite(glm::vec2 posInScreenSpace, glm::vec2 sizeInScreenSpace, TextureTile* textureTile, float zLayer = 0);

        // Render the batch, which means:
        // - update the VBO with all clipspace coordinates.
        // - (TODO) update the respective texture buffers, for now we have only one texture.
        void render();

        void clear();

        void updateSprite(int index, glm::vec2 screenPos, glm::vec2 sizeInScreenSpace);

    private:
        // This is used to store the clips space position data which
        // is used for glBufferData at rendering time.
        // During the frame, data is added via addSprite in screen space,
        // then these coordinates are transformed to clip space, where they are stored in this list.
        std::vector<float> _clipSpacePositions;
        std::vector<float> _uvs;
        std::vector<uint16_t> _indices;

        GLuint _vbo;
        GLuint _vao;
        GLuint _ibo;
        GLuint _uvbo;

        uint16_t _spriteNumber = 0;
    };

    struct UIButton {
        glm::vec2 pos;
        glm::vec2 size;
        std::string name;
        int spriteIndex = -1;
        SpriteBatch* spriteBatch = nullptr;

        glm::vec2 savedSize;

        void scaled(float val) {
            savedSize = size;
            spriteBatch->updateSprite(spriteIndex, pos, {size * val});
        }
        void unscaled() {
            size = savedSize;
            spriteBatch->updateSprite(spriteIndex, pos, savedSize);
        }
    };

    /**
     * This class provides methods to register
     * different UI elements such as buttons, textboxes etc.
     * Once per frame, the different query methods can be called to
     * know if e.g. a button has been pressed or text has been entered,
     * a combo-box item has been selected etc.
     */
    class UISystem  {

    public:
        void registerButton(glm::vec2 pos, glm::vec2 size, const std::string& name, gru::TextureTile* textureTile) {
            auto button = UIButton {pos, size, name};
            button.savedSize = size;
            if (_spriteBatch) {
                button.spriteIndex = (_spriteBatch->addSprite(pos, size, textureTile))-1;
                button.spriteBatch = _spriteBatch;

            }
            _buttons.push_back(button);
        }

        // Optionally register a spritebatch.
        // This will then automatically register the sprites for the buttons.
        void registerSpriteBatch(gru::SpriteBatch* spriteBatch) {
            _spriteBatch = spriteBatch;
        }

        // Returns the name of the hovered UIButton,
        // or an emtpy string if none was hovered.
        std::optional<UIButton> queryHovered(int mouseX, int mouseY) {
            std::optional<UIButton> hoveredButton;
            for (auto b : _buttons) {
                auto buttonLeft = b.pos.x - b.size.x/2;
                auto buttonRight = buttonLeft + b.size.x;
                auto buttonBottom = b.pos.y - b.size.y / 2;
                auto buttonTop = buttonBottom + b.size.y;
                if (mouseX >= buttonLeft && mouseX <= buttonRight
                        && mouseY >= buttonBottom && mouseY <= buttonTop) {
                    hoveredButton = b;
                    break;
                }
            }
            return hoveredButton;
        }

        std::vector<UIButton> buttons() { return _buttons; }

    private:



        std::vector<UIButton> _buttons;
        SpriteBatch* _spriteBatch = nullptr;

    };
}

struct MonitorResolution {
    uint64_t width;
    uint64_t height;
    uint64_t frequency;
    uint64_t bitsPerPixel;

};

struct GridData {
    float scale = 1;
    glm::vec4 color = {0.6, 0.8, 0.2, .5};
    glm::vec3 loc = {0, 0, 0};
    int numLines = 100;
    GLuint vao;
    Camera* camera = nullptr;
};



GridData* createGrid(int lines = 100);
GLuint createGridVAO(int lines = 100);
void drawText(const char* text, int x, int y, Bitmap* font);
void drawBitmap(int x, int y, Bitmap* bm);
void drawBitmapTile(int posx, int posy, int tileSize, int tilex, int tiley, Bitmap* bitmap, int offsetX  = 0, int offsetY = 0);
std::unique_ptr<Bitmap> loadBitmap(const char* fileName);
void setPixel(int x, int y, int r, int g, int b, int a);
void drawPlane();
void drawPlane(Light* directionalLight, const std::vector<Light*>& pointLights);
void drawMesh();
void drawMesh(const MeshDrawData& drawData);
void drawMeshIntoShadowMap(MeshDrawData dd, Light* l);
void drawSkybox();
void drawMeshSimple();
void drawMeshInstanced(int num);
void drawParticleEffect(bool instancedRender = false);
void drawGrid(GridData* gridData = {}, glm::ivec2 viewPortDimensions = glm::ivec2{scaled_width, scaled_height}, bool blurred = false);
void drawBitmap(int x, int y, uint8_t* bitmapPixels);
glm::mat4 getWorldMatrixFromGlobalState();
std::unique_ptr<Texture> createTextureFromFile(const std::string& fileName, GLenum colorFormat = GL_RGBA, GLint internalColorFormat = GL_SRGB8_ALPHA8);
std::unique_ptr<Texture> createTextureFromBitmap(Bitmap* bm, ColorFormat colorFormat = ColorFormat::RGBA);
std::unique_ptr<Texture> createCubeMapTextureFromDirectory(const std::string &dirName, ColorFormat colorFormat, const std::string& fileType = "png");
std::unique_ptr<FrameBuffer> createShadowMapFramebufferObject(glm::vec2 size);
void beginBatch();
void endBatch();
void foregroundColor(glm::vec4 col);

// This can be used with textures, will multiply with the texel color
void tint(glm::vec4 col);

std::shared_ptr<Texture> getDefaultNormalMap();
void scale(glm::vec3 val);
void textScale(glm::vec2 val);
void panUVS(glm::vec2 pan);
void location(glm::vec3 loc);
void rotation(glm::vec3 rot);
void overrideAlpha(float val);
void instanceOffsets(std::vector<glm::vec2> offsets);
void bindTexture(Texture* tex);
void bindNormalMap(Texture* tex, int unit = 2);
void bindSkyboxTexture(Texture* tex);
void unbindSkyboxTexture();
void tilingOn(bool val);
void tileData(int tileX, int tileY, int tileWidth, int tileHeight, int offsetX =0, int offsetY = 0);
void bindMesh(Mesh* mesh);
void bindParticleEffect(ParticleEffect* pe);
void renderTargetBitmap(Bitmap* bm);
void bindCamera(Camera* camera);
void bindShadowMapCamera(Camera* camera);
void font(Bitmap* fontBitmap);
std::vector<std::string> getAllActiveMonitors();
std::vector<MonitorResolution> getMonitorResolutions(const std::string& deviceName);
bool changeResolution(int width, int height, int refreshRate, const std::string& deviceName, bool goFullscreen = false);
void exitFullscreen();
void mouseVisible(bool val);
void lightingOn();
void lightingOff();
void shadowOff();
void shadowOn();
[[Deprecated]]
void wireframeOn(float lineWidth = 2.0f);
void wireframeOff();
void deferredStart();
void deferredEnd();
void flipUvs(bool val);
void uvScale(float val);
void forceShader(Shader* shader);
void setUniformFloat(int location, float val, Shader* shader = nullptr);
void updateParticleEffect(ParticleEffect* pe);
void resetParticleEffect(ParticleEffect* pe);
void bitmapBlitModeOn();
void bitmapBlitModeOff();
void depthTestOn();
void depthTestOff();
void useWorldMatrix(bool value);
void setWorldMatrix(glm::mat4 worldMatrix);
void setSkinnedDraw(bool value);
void setBoneMatrices(std::vector<glm::mat4> boneMatrices);
glm::mat4 calculateWorldTransform(Joint* j, glm::mat4 currentTransform);
GLuint createQuadVAO(PlanePivot pivot = PlanePivot::center);

glm::vec2 modelToScreenSpace(glm::vec3 model, glm::mat4 matWorld, Camera* camera);

std::unique_ptr<Texture> createEmptyTexture(int w, int h);
std::unique_ptr<Texture> createEmptyFloatTexture(int w, int h);
std::unique_ptr<Texture> createTextTexture(int w, int h);
void updateTexture(int w, int h, Texture* texture);
std::unique_ptr<Mesh> loadMeshFromFile(const std::string& fileName);
Result createShader(const std::string& vsrc, const std::string& fsrc, Shader* target);
Result createComputeShader(const std::string& source, Shader* target);
std::unique_ptr<Mesh> parseGLTF(JsonElement* gltfJson);
void prepareTransformationMatrices(glm::mat4 matworld, Camera* camera, Camera* shadowMapCamera, bool shadows);
void updateAndDrawText(const char *text, Texture *pTexture, int screenPosX, int screenPosY, int screenPosZ = -0.7);

// Creates a color framebuffer incl. depthbuffer
std::unique_ptr<FrameBuffer> createFrameBuffer(int width, int height, bool hdr = false, bool additionalColorBuffer = false);

// Creates a color framebuffer based on the given texture and a default depthtexture.
// An additional color texture can be used to create a second color attachment, e.g. for bloom effects.
std::unique_ptr<FrameBuffer> createFrameBufferWithTexture(int width, int height, std::shared_ptr<Texture> colorTexture, std::shared_ptr<Texture> colorTexture2 = nullptr);

bool keyPressed(int key);

void activateFrameBuffer(const FrameBuffer* fb);





/**
 * Produces a raytraced image of the given world which consists of
 * meshes.
 */
class Raytracer {
public:

    /**
     * Adds a model to the world
     * @param mesh
     */
    void addMesh(Mesh* mesh);

    /**
     * Render the world
     */
    void render(int pixelWidth, int pixelHeight);
    void rotate(glm::vec3 axis, float angleInRadians);

private:
    std::vector<Mesh*> _meshes;

};


enum class SceneNodeType {
    Light,
    Camera,
    Text,
    Mesh,
    ParticleSystem,
};

struct SceneMeshData {
    Mesh* mesh = nullptr;
    glm::vec4 color = glm::vec4(1.0f);
    Texture* texture = nullptr;
    Texture* normalMap = nullptr;
    Shader* shader = nullptr;
    glm::vec2 uvPan = {1, 1};
    glm::vec2 uvScale2 = {1, 1};
    float uvScale = 1;
    bool skinnedMesh = false;
    bool castShadow = true;
};
