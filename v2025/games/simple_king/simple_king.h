//
// Created by mgrus on 02.01.2024.
//

#ifndef SIMPLE_KING_SIMPLE_KING_H
#define SIMPLE_KING_SIMPLE_KING_H
#include <graphics.h>
#include <engine/sound/sound.h>
#include <map>
#include <vector>

enum class BuildState {
    None,
    House,
    Barrack,
    Storage,
};

struct StorageNode {
    int maxStorage = 20;
    int amount = 0;

};

struct BuildingData {
    float lastUnitSpawnTime = 0;
    int unitsSpawned = 0;
    int maxUnitsToSpawn = 4;
    BuildState buildState;
    glm::vec3 position;

    StorageNode* storageNode = nullptr;

    bool spawnsInGeneral() {
        return buildState == BuildState::House;
    }
};

enum class CitizenJobState {
    Jobless,
    Gatherer,
    Worker,
    Soldier
};

enum class CitizenOccupationState {
    Clueless,
    Walking,
    Running,
    Crouching,
    Flying,
    Swimming,
    Climbing,
    Gathering,
    Loading,
    Unloading,
    FinishedLastJob,
};

enum class MoveTargetType {
    RandomWanderLocation,
    ResourceNode,
    StorageNode,
    EnemyPosition
};

enum class ResourceType {
    Wood,
    Stone,
    Iron,
    Wool,
    Brick,
    Trouser,
    Shirt,
    Gun,
    Sword,
    Animal,
    Meat,
    Gold

};


struct Citizen {
    glm::vec3 nextMoveTargetPosition;
    glm::vec3 position;

    bool hasMoveTarget = false;
    void* moveTarget = nullptr;
    void* jobNode = nullptr;

    MoveTargetType moveTargetType = MoveTargetType::RandomWanderLocation;
    CitizenJobState jobState = CitizenJobState::Gatherer;
    CitizenOccupationState occupationState = CitizenOccupationState::Clueless;
    float lastOccupationStartTime = 0;
    int resourcesCarriedAmount = 0;
    ResourceType resourcesCarriedType;


};


struct ResourceNode {
    ResourceType resourceType;
    int amount = 1000;                  // how much resources are at this spot?
    int amountPerGathering = 10;        // how much does a gather get in one round?
    glm::vec3 position;
    int maxGatherers = 4;
    std::vector<Citizen*> gatherers;
    float gatheringTime = 8;

    void assignGatherer(Citizen* cit) {
        if (gatherers.size() < maxGatherers) {
            gatherers.push_back(cit);
            cit->jobNode = this;
        }
    }

    bool hasJob() {
        return gatherers.size() < maxGatherers;
    }

};


struct Resources {
    int wood = 100;
    int stone = 100;
    int gold = 100;
    int citizens = 0;
};




struct GameState {
    Camera* uiCamera;
    Camera* gamePlayCamera;
    Camera* debugCamera;
    glm::vec3 gamePlayLookFwd = {-4, -12, -3};
    Mesh* cubeMesh;
    Mesh* houseMesh;
    Mesh* barrackMesh;
    Mesh* islandMesh;
    Mesh *stoneNodeMesh;
    Mesh* storageNodeMesh;
    Mesh* treeNodeMesh = nullptr;
    Mesh *planeMesh = nullptr;
    Mesh *maleFarmerMesh = nullptr;
    std::map<std::string, Sound*> sounds;
    std::map<std::string, Texture*> textures;
    int buttonHoverIndex = -1;
    glm::vec3 lastRayHitPoint;
    BuildState buildState = BuildState::None;
    //std::vector<glm::vec3> buildingLocations;
    std::vector<BuildingData*> buildingData;
    std::vector<Citizen*> citizens;
    std::vector<ResourceNode*> resourceNodes;
    Resources resources;
    bool occupiedRegister[100*100];
    bool placementAllowed = false;
    float buildButtonsLeftOffset = window_width - 40;
    bool shouldRun = true;



};


#endif //SIMPLE_KING_SIMPLE_KING_H
