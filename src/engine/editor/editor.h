#pragma once
#include <engine/game/default_game.h>
#include <engine/animation/AnimationPlayer.h>

#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/base/containers/vector.h"
#include "ozz/include/ozz/base/maths/transform.h"

namespace ozz::math {
    struct Float4x4;
    struct SoaTransform;
}

class Level;
class GameObject;
class FolderAssetLoader;
class CameraMover;


namespace editor {

    struct EditorState {
        bool editingLevel = false;
    };

    class Editor {

    public:
        Editor();
        ~Editor();

        void renderImGui();
        void update();

    private:
        EditorState* state = nullptr;
        //Texture* playButtonTexture = nullptr;
        Level* level;
        GameObject* currentSelectedGameObjectInTree = nullptr;
        FolderAssetLoader *assetLoader = nullptr;
        std::unique_ptr<FrameBuffer> skeletalMeshWindowFrameBuffer = nullptr;
        Animation* currentAnimation = nullptr;
        std::shared_ptr<ozz::animation::Animation> currentOzzAnimation;
        float blendTimestamp = 0;
        float blendWeight = 0.5;
        int currentAnimationFrame = 0;
        bool animationLooped = false;
        bool animationPlaying = false;
        Camera* _meshViewerCamera = nullptr;
        Mesh* importedMesh = nullptr;
        std::unique_ptr<Texture> importedTexture = nullptr;
        std::map<std::string, Animation*> importedAnimations;
        CameraMover* cameraMover = nullptr;
        std::string lastImportedMeshFileName = "";
        std::string lastImporteTextureFileName = "";
        std::string lastImportedAnimationFileName = "";
        AnimationPlayer* animationPlayer = nullptr;
        Shader * staticMeshShader = nullptr;
        Shader* skinnedMeshShader = nullptr;
        Shader* gradientShader = nullptr;
        std::unordered_set<Animation*> blendedAnimations;


    private:
        void drawAnimationTimeline();
        void drawTopToolbar();

        void drawAnimationOverview();

        void ShowGameEditor();
        void recurseRenderGameObjects(GameObject *parent);
        void renderMainMenu();

        std::string selectFolderUsingIFileDialog();

        void renderGameObjectsMenu();

        void doImportMeshAction();

        void renderMeshMenu();
        void renderMeshViewer();
        std::string showFileDialog(const std::string& typeFilter);
        void renderMouseCoordinates();
        void saveLevel();
        Camera *getMeshViewerCamera();

        ozz::math::Transform convertSoaToAos(const ozz::math::SoaTransform &soa_transform, int element);

        void renderMeshViewerExt();

        std::string skeletonBaseName = "";

        // Sampling context.
        ozz::animation::SamplingJob::Context context_;

        // Buffer of local transforms as sampled from animation_.
        ozz::vector<ozz::math::SoaTransform> locals_;

        // Buffer of model space matrices.
        ozz::vector<ozz::math::Float4x4> models_;

        int window_height = 720;
        int window_width = 1280;


    };

/**
 * EditorGame is technically also just a DefaultGame.
 * This allows to re-use the tools and utils we have for games.
 * Using the same tools, we test them along the way.
 *
 */
    class EditorGame : public DefaultGame {
    public:
        ~EditorGame();
        void update() override;
        void init() override;
        void render() override;
        bool shouldStillRun() override;

    private:
        Editor* editor = nullptr;
    };

}
