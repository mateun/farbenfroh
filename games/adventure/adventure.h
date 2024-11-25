//
// Created by mgrus on 07.06.2024.
//

#ifndef SIMPLE_KING_ADVENTURE_H
#define SIMPLE_KING_ADVENTURE_H

namespace adv {

    struct GameObject {
        int id = -1;
        int hp = 100;
        int mana = 50;
        glm::vec2 screenPos;
        glm::vec2 sizeInPixels;
    };

    struct GameState {
        std::vector<GameObject>* gameObjects = nullptr;
        Texture *texTileMap = nullptr;
    };

    class Adventure : public DefaultGame {

    public:
        Adventure();

        void update() override;
        void render() override;
        bool shouldStillRun() override;
        void init() override;
        std::string getAssetFolder() override;

        bool shouldRun();

    private:
        void initWalls();

    private:
        GameState* _gameState = nullptr;
    };

} // adv

#endif //SIMPLE_KING_ADVENTURE_H
