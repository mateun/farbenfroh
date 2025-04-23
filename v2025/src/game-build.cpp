// This file includes every cpp file which is needed for the game to build.
// Technically, from the point of view of CMake, we only build this file.
// We do not need to update CMakeLists.txt very often, only adding file to compile here.

#include <windows.cpp>
#include <engine/io/io.cpp>
#include <engine/io/json.cpp>
#include <engine/sound/sound.cpp>
#include <engine/math/math3d.cpp>
#include <engine/game/game_model.cpp>
#include <engine/algo/Bvh.cpp>
#include <engine/rastergraphics/rastergraphics.cpp>
#include <engine/animation/Cinematic.cpp>
#include <engine/ui/Window.cpp>
#include <engine/input/UpdateSwitcher.cpp>
#include <engine/vulkan/VulkanRenderer.cpp>
#include <engine/lighting/Light.cpp>
#include <graphics.cpp>
#include <engine/physics/PhysicsSystem.cpp>
#include <engine/physics/JoltPhysicsSystem.cpp>
#include <engine/animation/CharacterController.cpp>
#include <engine/animation/Animation.cpp>
#include <engine/animation/AnimationBlender.cpp>
#include <engine/animation/AnimationState.cpp>
#include <engine/animation/AnimationController.cpp>
#include <engine/animation/AnimationTransition.cpp>
#include <engine/animation/AnimationPlayer.cpp>
#include <engine/animation/Pose.cpp>
#include <engine/animation/JointMask.cpp>
#include <engine/animation/PerBoneBlendData.cpp>
#include <engine/animation/BoneMatrixCalculator.cpp>
#include <engine/algo/VectorUtils.cpp>
#include <engine/algo/QuadTree.cpp>
#include <engine/algo/CollisionGrid.cpp>
#include <engine/fx/ParticleSystem.cpp>
#include <engine/compute/ComputeShader.cpp>
#include <engine/profiling/PerformanceTimer.cpp>
#include <engine/game/SceneNode.cpp>
#include <engine/game/Scene.cpp>
#include <engine/game/NodeComponent.cpp>
#include <engine/io/AssimpMeshImporter.cpp>
#include <engine/io/OzzSkeletonImporter.cpp>
#include <engine/ui/Application.cpp>
#include <engine/ui/Widget.cpp>
#include <engine/ui/Camera.cpp>


//#include "../games/simple_king/king_main.cpp"

// Mars attack
//#include "../games/mars_attack/physics.cpp"
//#include "../games/mars_attack/animation_editor.cpp"
//#include "../games/mars_attack/mars_main_menu.cpp"
//#include "../games/mars_attack/ingame_editor.cpp"
//#include "../games/mars_attack/behavior_tree.cpp"
//#include "../games/mars_attack/mars_main.cpp"

// mtking
//#include "../games/multithread-king/mtking-main.cpp"
//#include "../games/multithread-king/WallCollider.cpp"
//#include "../games/multithread-king/FirstPersonController.cpp"
//#include "../games/multithread-king/physics.cpp"

// Default Game
#include <engine/game/default_game.cpp>
#include <engine/game/FolderAssetLoader.cpp>

// Physics
#include <engine/physics/Particle.cpp>

// Editor
#include <engine/editor/editor.cpp>

// Terrain
#include <engine/terrain/terrain.cpp>
#include <engine/terrain/planet.cpp>


// Adventure
//#include "../games/adventure/adventure.cpp"

// Dungeon Shoot
//#include "../games/dungeon_shoot/dungeon_shoot_main.cpp"

// Vulkan/Shmup
//#include "../games/vulkan/shmup/Shmup.cpp"


// Opengl/ServerViz
//#include "../games/opengl/server_viz/ServerViz.cpp"

// Opengl/Testgame1
// #include "../games/opengl/testgame1/TestGame1.cpp"
// #include "../games/opengl/testgame1/MainMenuLevel.cpp"
// #include "../games/opengl/testgame1/GamePlayLevel.cpp"

// Opengl/TerrainTest
#include "../games/opengl/test_terrain/TestTerrainGame.cpp"
#include "../games/opengl/test_terrain/GameplayLevel.cpp"
#include "../games/opengl/test_terrain/LevelEditor.cpp"
#include "../games/opengl/test_terrain/PlayerBulletComp.cpp"
#include "../games/opengl/test_terrain/EnemyHitManager.cpp"
#include "../games/opengl/test_terrain/PlayerShooting.cpp"
#include "../games/opengl/test_terrain/PlayerBulletPool.cpp"
#include "../games/opengl/test_terrain/CollisionManager.cpp"
#include "../games/opengl/test_terrain/EnemyBulletPool.cpp"
#include "../games/opengl/test_terrain/AnimationEditor.cpp"

//#include "../src/engine/editor/shadergraph/ShaderGraph.cpp"

// TopDownMovement
//#include "../games/topdown_mover/TopDownGame.cpp"


// Software rendered king
//#include "../games/software_rendered_king/SoftwareRenderedGame.cpp"

// King 1024
//#include "../games/king1024/king_game.cpp"
//#include "../games/king1024/InsideShipState.cpp"
//#include "../games/king1024/MainMenuState.cpp"
//#include "../games/king1024/BuildingGameplay.cpp"
//#include "../games/king1024/SettingsState.cpp"
//#include "../games/king1024/physics.cpp"

// Car
//#include "../games/car/car_main.cpp"
//#include "../games/car/splash.cpp"
//#include "../games/car/model_import.cpp"
//#include "../games/car/car.cpp"

