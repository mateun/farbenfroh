//
// Created by mgrus on 02.01.2024.
//

#ifndef SIMPLE_KING_MARS_ATTACK_H
#define SIMPLE_KING_MARS_ATTACK_H
#include <map>
#include <vector>
#include "behavior_tree.h"

struct Obstacle {
    glm::vec3 location;
    glm::vec3 dimensionAABB;
    bool destructable = false;

    // This indicates that this obstacle was hit in this frame
    // We may render accordingly some effect etc.
    bool markAsHit = false;

    // To be removed in the next frame.
    bool toRemove = false;
    Mesh * mesh = nullptr;
    Texture *texture = nullptr;
    glm::vec4 singleColor = glm::vec4(1, 1, 1, 1);
    glm::vec3 rotation = glm::vec3(0, 0, 0);
};

enum class EnemyType {
    Worm,
    Spider,
    Marauder,
    Squirrel
};

struct Enemy {
    EnemyType type = EnemyType::Worm;
    glm::vec3 location;
    glm::vec3 rotation = glm::vec3(0, 0, 0);
    glm::vec3 dimensionAABB;
    int hp = 100;
    BehaviorTree* behaviorTree = nullptr;
    bool markAsHit = false;
};

struct Projectile {
    std::string id;
    glm::vec3 location;
    glm::vec3 velocity;
    glm::vec3 rotation;
    float damage = 10;
    float lifeTime = 0;
    float maxLife = 2;
    bool markDelete = false;
    bool markHit = false;
};

enum class TutorialState {
    UPDOWN,
    LEFTRIGHT,
    TURRETTURN,
    SHOOT,
    OVER
};

struct Tutorial {
    TutorialState state = TutorialState::UPDOWN;
    std::string movUpDownText = "Press 'W' for throttle and 'S' to brake!";
    std::string moveLeftRightText = "Press 'A' and 'D' to turn tank left and right!";
    std::string turretTurnText = "Press 'LEFT' and 'RIGHT' cursor to turn the turret!";
    std::string shootText = "Press 'SPACE' to shoot!";
    bool moveUpDownRecognised = false;
    bool moveLeftRightRecognised = false;
    bool turnSpotRecognised = false;
    bool turretTurnRecognised = false;
    bool shootRecognised = false;
};

enum class Level {
    MainMenu,
    PauseMenu,
    AnimEditor,
    HostGame,
    JoinGame,
    Lobby,
    Gameplay,
    Outro,
    Settings,
    InGameEditor,
};

enum class GameObjectType {
    Generic,
    Enemy,
    Obstacle,
    Player,
    Tank,
    Turret,
    Projectile
};








// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
//class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
//{
//public:
//    BPLayerInterfaceImpl()
//    {
//        // Create a mapping table from object to broad phase layer
//        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
//        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
//    }
//
//    virtual uint					GetNumBroadPhaseLayers() const override
//    {
//        return BroadPhaseLayers::NUM_LAYERS;
//    }
//
//    virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
//    {
//        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
//        return mObjectToBroadPhase[inLayer];
//    }
//
//#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
//    virtual const char *			GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
//	{
//		switch ((BroadPhaseLayer::Type)inLayer)
//		{
//		case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
//		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
//		default:													JPH_ASSERT(false); return "INVALID";
//		}
//	}
//#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
//
//private:
//    BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
//};




struct GameObject {
    std::string name;
    GameObjectType type = GameObjectType::Generic;
    glm::vec3 location;
    // This is a pointer to an actual object, like an Enemy, or
    // obstacle etc.
    // Other code may use this pointer to actually inspect non generic data.
    // Otherweise we rely on every part of the code to actually update the
    // GameObject portion. A little bit implementing polymorphism again.
    void* object = nullptr;
    bool usePhysics = false;
    JPH::BodyID physicsBodyId;
    JPH::Body* physicsBody = nullptr;

};

struct GameState {
    bool shouldRun = true;
    Camera *gamePlayCamera = nullptr;
    Camera* shadowMapCamera = nullptr;
    glm::vec3 shadowCamOffsetToNormalCam = {12, -1, 1};
    Camera *uiCamera = nullptr;

    PhysicsData* physicsData = nullptr;

    glm::vec3 gamePlayLookFwd = {0, -4, -1.5};
    std::map<std::string, Mesh*> meshes;
    std::map<std::string, Texture*> textures;
    std::map<std::string, Sound*> sounds;

    std::vector<Obstacle*> obstacles;
    std::vector<Enemy*> enemies;
    std::vector<Projectile*> projectiles;


    // Memorize the last n tank positions so we can render
    // tread marks on the ground.
    std::vector<glm::vec3> tankPositions;

    // Tank attributes
    float tankSpeed = 0;
    float tankAccel = 0;
    float throttlePercent = 0;
    float brakePercent = 0;
    glm::vec3 tankLocation;
    glm::vec3 turretLocation;
    glm::vec3 turretLocationOffset;
    float tankYaw;
    float turretYaw = 0;


    Bitmap* font = nullptr;

    Tutorial* tutorial = nullptr;

    std::vector<glm::vec2> stoneInstanceOffsets;
    std::map<std::string, ParticleEffect*> particleEffects;

    float recoilAnimDuration = 0.0f;
    float recoilAnimMaxDuration = 0.1f;
    bool doTurretRecoil = false;

    Level level = Level::MainMenu;

    std::vector<GameObject*> allGameObjects;
    // Pause all updates, only have debug updates active
    // e.g. for debug cam etc.
    bool paused = false;
};

#endif //SIMPLE_KING_MARS_ATTACK_H
