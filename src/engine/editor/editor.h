#pragma once
#include <engine/animation/AnimationPlayer.h>

class Level;
class GameObject;

namespace editor {

    struct EditorState {
        bool editingLevel = false;
    };

    class Editor {

    public:
        Editor();

    public:
        void renderImGui();

        void update();

    private:
        EditorState* state = nullptr;
        //Texture* playButtonTexture = nullptr;
        Level* level;
        GameObject* currentSelectedGameObjectInTree = nullptr;
        FolderAssetLoader *assetLoader = nullptr;
        FrameBuffer* skeletalMeshWindowFrameBuffer = nullptr;
        Animation* currentAnimation = nullptr;
        int currentAnimationFrame = 0;
        bool animationLooped = false;
        bool animationPlaying = false;
        Camera* _meshViewerCamera = nullptr;
        Mesh* importedMesh = nullptr;
        Texture* importedTexture = nullptr;
        std::map<std::string, Animation*> importedAnimations;
        CameraMover* cameraMover = nullptr;
        std::string lastImportedMeshFileName = "";
        std::string lastImporteTextureFileName = "";
        std::string lastImportedAnimationFileName = "";
        AnimationPlayer* animationPlayer = nullptr;
        Shader * staticMeshShader = nullptr;
        Shader* skinnedMeshShader = nullptr;

    private:
        void drawAnimationTimeline();
        void drawTopToolbar();
        void ShowGameEditor();
        void recurseRenderGameObjects(GameObject *parent);
        void renderMainMenu();
        void renderGameObjectsMenu();

        void doImportMeshAction();

        void renderMeshMenu();
        void renderMeshViewer();
        std::string showFileDialog(const std::string& typeFilter);
        void renderMouseCoordinates();
        void saveLevel();
        Camera *getMeshViewerCamera();

        void renderMeshViewerExt();
    };

/**
 * EditorGame is technically also just a DefaultGame.
 * This allows to re-use the tools and utils we have for games.
 * Using the same tools, we test them along the way.
 *
 */
    class EditorGame : public DefaultApp {
    public:
        void update() override;
        void init() override;
        void render() override;
        bool shouldStillRun() override;

    private:
        Editor* editor = nullptr;
    };

}
