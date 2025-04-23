//
// Created by mgrus on 01.11.2024.
//

#ifndef SIMPLE_KING_BUILDINGGAMEPLAY_H
#define SIMPLE_KING_BUILDINGGAMEPLAY_H

#include "king_game.h"

namespace  king {

    enum class EGamePlayMode {
        Normal,
        BuildingPlacement,
        BuildingSelection,
        StatisticsPopup,
        Settings,

    };

    enum class BuildingType {
        House,
        Barrack,
        Archery,
        Blacksmith,
        Keep,
        Library,
        Lumbermill,
        Market,
        Stables,
        Temple,
        Tower,
        Townhall,
        Wall,
        Street
    };

    struct Building {
        BuildingType type;
        glm::vec2 mapLocation = {-1, -1};
        int creationCost = 50;
        int maintCost = 0;

    };

    class BuildingGameplay : public KingGameState {

    public:

        BuildingGameplay(KingGame* game);

        void update() override;
        void render() override;
        void init() override;


        glm::vec3 cursorLocation;
        Texture *terrainTexture = nullptr;
        uint8_t *terrainType = nullptr;

    private:
        EGamePlayMode mode = EGamePlayMode::Normal;
        int selBuildingIndex = 0;
        glm::ivec2 terrainSize = {100, 100};

        std::vector<Building*> buildings;

        void resetCursorToCurrentView();

        void onUserPlacedBuilding();

        bool costCheck(BuildingType buildingType);

        // We store a picture of every building in a texture
        Texture *housePreRenderedTexture = nullptr;
        FrameBuffer *preRenderFrambebuffer = nullptr;
    };
}




#endif //SIMPLE_KING_BUILDINGGAMEPLAY_H
