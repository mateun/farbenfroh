#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <optional>
#include <GL/glew.h>
#include <engine/input/UpdateSwitcher.h>
#include <engine/lighting/Light.h>
#include <glm/glm.hpp>
#include "glm/detail/type_quat.hpp"
#include "assimp/scene.h"
#include "../src/engine/game/game_model.h"
#include "../src/engine/animation/Animation.h"

#define GL_ERROR_EXIT(code)   auto err_##code = glGetError(); \
                        if (err_##code != 0) { \
                            exit(code); \
                        }

#define GL_ERROR_EXIT_INFO(code, info)   auto err_##code = glGetError(); \
                                    if (err_##code != 0) { \
                                        printf("GL error %d: \ncontext info: %s\n", err_##code, info.c_str()); \
                                        exit(code); \
                                    }

extern int scaled_width, scaled_height;
class JsonElement;

struct BITMAP_FILE {
	BITMAPFILEHEADER bmfileHeader;
	BITMAPINFOHEADER bminfoHeader;

};

struct Bitmap {
	int width=-1;
	int height=-1;
	uint8_t* pixels=nullptr;
};

enum class PlanePivot {
    topleft,
    bottomleft,
    topright,
    bottomright,
    center,
};


struct Shader {
    GLuint handle;

    void setFloatValue(float uv_scale, const std::string& name);
    void setVec3Value(glm::vec<3, float> vec, const std::string& name);
    void setVec4Value(const glm::vec4 vec, const std::string& name);
    void setMat4Value(glm::mat4 mat, const std::string& str);
    void setMat4Array(const std::vector<glm::mat4> mats, const std::string& name);

    void initFromFiles(const std::string& vertexShader, const std::string& fragmentShader);


};

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
    float maxLength = 100;
};

struct Result {
    bool ok;
    std::vector<std::string> errors;
};

struct Texture {
    GLuint handle;
    Bitmap* bitmap;

    void bindAt(int unitIndex);
};

class FBFont {
public:
    FBFont(const std::string& fileName);
    void renderText(const std::string& text, glm::vec3 position);

private:
    Texture* texture = nullptr;
    Bitmap* bitmap = nullptr;
};





struct Joint {
    std::string name;
    glm::mat4 inverseBindMatrix;

    // These are the transformations in parent space
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;
    glm::mat4 localTransform= glm::mat4(1);
    glm::mat4 globalTransform = glm::mat4(1);
    // This is pre-multiplied the global and inverseBind matrix:
    glm::mat4 finalTransform = glm::mat4(1);

    // These are the transformations as matrices in model space
    glm::mat4 modelTranslation = glm::mat4(1);
    glm::mat4 modelRotation = glm::mat4(1);
    glm::mat4 modelScale = glm::mat4(1);
    std::vector<Joint*> children;
    Joint* parent = nullptr;


};

struct Skeleton {
    std::vector<Joint*> joints;
};

/**
 * A wrapper around an OpenGL Framebuffer
 */
struct FrameBuffer {
    GLuint handle;
    Texture* texture;
};

/**
 * A 3D mesh which can be rendered anywhere
 * in the world.
 * Bind it and then it can be rendered.
 */
struct Mesh {
    GLuint vao = 0;
    GLuint instanceOffsetVBO = 0;
    GLuint instanceMatrixVBO = 0;
    GLuint instanceColorVBO = 0;
    GLuint instanceTintVBO = 0;
    int numberOfIndices = -1;
    GLenum indexDataType = GL_UNSIGNED_INT;

    Skeleton* skeleton = nullptr;

    std::vector<glm::vec3> positions;
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> normals;
    std::vector<glm::ivec4> boneIndices;
    std::vector<glm::vec4> boneWeights;
    std::string fileName;

    std::vector<Animation*> animations;

    bool rayCollides(Ray ray, glm::vec4& color);
    Animation* findAnimation(const std::string& name);
};

enum class CameraType {
    Ortho,
    Perspective,
    OrthoGameplay,
};


struct Camera {
    CameraType type;
    glm::vec3 location;
    glm::vec3 lookAtTarget;
    GameObject* followedObject = nullptr;
    glm::vec3 followOffset;
    glm::vec3 followDirection;

    void follow(GameObject* gameObject, glm::vec3 offset) {
        this->followedObject = gameObject;
        this->followOffset = offset;
        this->followDirection = glm::normalize(gameObject->location - location);
    }

    void updateFollow() {
        if (followedObject) {
            updateLocation(followedObject->location + followOffset);
            this->followDirection = glm::normalize(followedObject->location - location);
            updateLookupTarget(followedObject->location);
        }
    }

    void updateLocation(glm::vec3 loc) {
       location = loc;
    }

    void updateLookupTarget(glm::vec3 t) {
        lookAtTarget = t;
    }

    glm::mat4 getViewMatrix() {
        return lookAt((location), (lookAtTarget), glm::vec3(0, 1,0));
    }

    glm::mat4 getLightProjectionMaatrix() {
        // Directional light
        if (type == CameraType::Ortho) {
            return glm::ortho<float>(0, scaled_width, 0, scaled_height, 0.1f, 200);
        }

        // Assume perspective
        return glm::ortho<float>(-10, 10, -8, 8, 0.1f, 400);

    }

    glm::mat4 getProjectionMatrix(std::optional<glm::ivec2> widthHeightOverride = std::nullopt, std::optional<float> fovOverride = 50.0f) {

        auto w = scaled_width;
        auto h = scaled_height;
        if (widthHeightOverride.has_value()) {
            w = widthHeightOverride.value().x;
            h = widthHeightOverride.value().y;
        }

        float fov = 50;
        if (fovOverride.has_value()) {
            fov = fovOverride.value();
        }

        if (type == CameraType::Ortho) {
            return glm::ortho<float>(0, w, 0, h, 0.1f, 200);
        }

        if (type == CameraType::OrthoGameplay) {
            return glm::ortho<float>(-10, 10, -8, 8, 0.1f, 400);
        }

        if (type == CameraType::Perspective) {
            return glm::perspectiveFov<float>(glm::radians(fov), w, h, 0.1f, 1000);
        }
    }

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

/**
 * CameraMove can process inputs to move a camera around.
 * Mainly useable for debug cameras with classical WASD movement scheme.
 */
class CameraMover : public Updatable {

public:
    CameraMover(Camera* cam, CameraCollider* cameraCollider = nullptr);
    void update() override;
    void setMovementSpeed(float val);
    void setFixedPlaneForwardMovement(bool b);

    // E.g. to avoid pitch clipping and being lost
    void reset();

private:
    Camera* _cam;
    CameraCollider* cameraCollider;

    float movementSpeed = 5;
    bool fixedPlaneFwdMovement = false;
    bool clampPitch = false;
    glm::vec3 originalLocation;
    glm::vec3 originalTarget;
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
    FrameBuffer* renderToFrameBuffer = nullptr;
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

//glm::vec3 lightDirection{0.6, -2, -0.5};

/**
* This holds everything we need to issue a physical draw call.
* A lot ... but what can you do.
*/
struct MeshDrawData {
    glm::vec3 location = {0, 0, 0};
    glm::vec3 scale = {1, 1, 1};
    glm::vec3 rotationEulers = {0, 0, 0};
    std::vector<glm::mat4> boneMatrices;
    std::optional<glm::mat4> worldTransform;
    Mesh* mesh = nullptr;
    Shader* shader = nullptr;
    Camera* camera = nullptr;
    Texture* texture = nullptr;
    Texture* normalMap = nullptr;
    glm::vec4 color = {1, 0, 1, 1}; // Nice pink if we have no texture set.
    Light* directionalLight = nullptr;
    float uvScale = 1;
    bool depthTest = true;
    bool skinnedDraw = false;
    std::optional<glm::ivec2> viewPortDimensions;
};

GridData* createGrid(int lines = 100);
GLuint createGridVAO(int lines = 100);
void drawText(const char* text, int x, int y, Bitmap* font);
void drawBitmap(int x, int y, Bitmap* bm);
void drawBitmapTile(int posx, int posy, int tileSize, int tilex, int tiley, Bitmap* bitmap, int offsetX  = 0, int offsetY = 0);
void loadBitmap(const char* fileName, Bitmap** bm);
void setPixel(int x, int y, int r, int g, int b, int a);
void drawPlane();
void drawPlane(Light* directionalLight, const std::vector<Light*>& pointLights);
void drawMesh();
void drawMesh(const MeshDrawData& drawData);
void drawMeshIntoShadowMap(FrameBuffer* shadowMapFBO);
void drawSkybox();
void drawMeshSimple();
void drawMeshInstanced(int num);
void drawParticleEffect(bool instancedRender = false);
void drawGrid(GridData* gridData = {}, glm::ivec2 viewPortDimensions = glm::ivec2{scaled_width, scaled_height}, bool blurred = false);
void drawBitmap(int x, int y, uint8_t* bitmapPixels);
glm::mat4 getWorldMatrixFromGlobalState();
Texture* createTextureFromFile(const std::string& fileName, ColorFormat colorFormat = ColorFormat::RGBA, ColorFormat internalColorFormat = ColorFormat::RGBA);
Texture* createTextureFromBitmap(Bitmap* bm, ColorFormat colorFormat = ColorFormat::RGBA);
Texture* createCubeMapTextureFromDirectory(const std::string &dirName, ColorFormat colorFormat, const std::string& fileType = "png");
void beginBatch();
void endBatch();
void foregroundColor(glm::vec4 col);

// This can be used with textures, will multiply with the texel color
void tint(glm::vec4 col);

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
bool changeResolution(int width, int height, int refreshRate, const std::string& deviceName);
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

Texture* createEmptyTexture(int w, int h);
Texture* createTextTexture(int w, int h);
Mesh* loadMeshFromFile(const std::string& fileName);
Result createShader(const std::string& vsrc, const std::string& fsrc, Shader* target);
Result createComputeShader(const std::string& source, Shader* target);
Mesh* parseGLTF(JsonElement* gltfJson);
void prepareTransformationMatrices(glm::mat4 matworld, Camera* camera, Camera* shadowMapCamera, bool shadows);
void updateAndDrawText(const char *text, Texture *pTexture, int screenPosX, int screenPosY, int screenPosZ = -0.7);

// createFrameBuffer creates a color framebuffer incl. depthbuffer
FrameBuffer* createFrameBuffer(int width, int height);

void activateFrameBuffer(FrameBuffer* fb);


class MeshImporter {
public:

    /**
     * FBX notes:
     * For .fbx files out of Blender, first rotate the object by -90 degress around the x-axis.
     * Then apply all transforms.
     * Then export fbx.
     *
     * GLTF notes:
     * Disable "Sample optimization" and "Keep channel..." (or similar settings).
     * This will lead to exact non optimized frame information, making it easier to debug.
     *
    **/
    Mesh* importMesh(const std::string& filePath, bool debugPrintSM = false);
    std::vector<Animation*> importAnimations(const std::string& filePath);

private:
    std::vector<Animation*> importAnimationsInternal(const aiScene* scene);
};

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
    Mesh
};

class SceneNode {
public:
    SceneNode();
    ~SceneNode();

    void yaw(float degrees);

    glm::vec3 getForwardVector();
    glm::vec3 getRightVector();
    glm::vec3 getVelocity();

    SceneNodeType type;

    // These are type specific fields and can be null
    Mesh* mesh = nullptr;
    Texture* texture = nullptr;
    Texture* normalMap = nullptr;
    Shader* shader = nullptr;

    glm::vec3 location = glm::vec3(0);
    glm::vec3 velocity = glm::vec3(0);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 rotation = glm::vec3(0);
    glm::vec3 forward= {0, 0, -1};   // This is normally how we face when coming from Blender
    glm::vec3 right = {1, 0, 0}; // Based on the incoming Blender default orientation
    glm::vec4 foregroundColor = {1, 0,1, 1};
    float uvScale = 1;
    bool skinnedMesh = false;
    std::vector<glm::mat4> boneMatrices;

};

class Scene {
public:
    Scene();
    ~Scene();

    void setCamera(Camera *camera);

    void addNode(SceneNode* node);
    void setDirectionalLight(Light* light);
    void update();
    void render();

private:
    Light* directionalLight;
    std::vector<Light*> pointLights;
    Camera* gameplayCamera = nullptr;
    Camera* uiCamera = nullptr;

    std::vector<SceneNode*> meshNodes;
    std::vector<SceneNode*> textNodes;

};