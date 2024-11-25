//
// Created by mgrus on 07.06.2024.
//

#ifndef SIMPLE_KING_ADVENTURE_H
#define SIMPLE_KING_ADVENTURE_H

#include "splash.h"
#include <map>

namespace cargame {

    enum class GameState {
        splash,
        main_menu,
        ingame,
        game_won,
        game_lost
    };

    struct Model {
        std::vector<DirectX::XMFLOAT3> positions;
        std::vector<DirectX::XMFLOAT2> uvs;
        std::vector<DirectX::XMFLOAT3> normals;
        std::vector<unsigned int> indices;

        ID3D11Buffer *matrixBuffer = nullptr;
        ID3D11Buffer *vertexBuffer = nullptr;
        ID3D11Buffer *uvBuffer = nullptr;
        ID3D11Buffer *normalBuffer = nullptr;
        ID3D11Buffer *indexBuffer = nullptr;
        ID3D11Texture2D *tex = nullptr;
        ID3D11SamplerState* samplerState = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;
        ID3D11RasterizerState* rs = nullptr;
    };




    class SceneNode {

    public:
        void setModel(Model* model);
        Model *getModel();
        DirectX::XMFLOAT3 getPosition();
        void setPosition(float x, float y, float z);
        void setRasterizerState(ID3D11RasterizerState* rs);
        ID3D11RasterizerState *getRasterizerState();
        bool shouldRender();

        DirectX::XMFLOAT3 getScale();
        void setScale(float x, float y, float z);

        DirectX::XMFLOAT2 getTextureScale();
        void setTextureScale(float x, float y);

        void setOrientation(DirectX::XMFLOAT3 eulers);
        DirectX::XMFLOAT3 getOrientation();

    private:
        Model* _model = nullptr;
        DirectX::XMFLOAT3 _position = {0, 0, 0};
        DirectX::XMFLOAT3 _scale = {1, 1, 1};
        DirectX::XMFLOAT3 _orientation = {0, 0, 0};
        DirectX::XMFLOAT2 _textureScale = {1, 1};
        ID3D11RasterizerState * _rs = nullptr;

    };

    class Scene {
    public:
        void addNode(SceneNode* n);
        void render();

    private:
        std::vector<SceneNode*> _nodes;
    };

    struct GameData {
        GameState state = GameState::splash;
        Splash splash;
        int score;

        cargame::Model *carModel = nullptr;
        cargame::Model *groundPlaneModel = nullptr;
        cargame::Model *houseModel = nullptr;
        cargame::Model *trackModel = nullptr;

        cargame::Scene *mainMenuScene;

        ID3D11RasterizerState* rsWireFrame = nullptr;
        ID3D11RasterizerState* rsSolid = nullptr;

        ID3D11Texture2D *xboxTexture = nullptr;
        ID3D11Texture2D *roadTexture = nullptr;

        HWND hwnd;
    };

}

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

    class Car : public DefaultGame {

    public:
        Car();

        void update() override;
        void render() override;
        bool shouldStillRun() override;
        void init() override;
        std::string getAssetFolder() override;
        Camera* getGameplayCamera() override;

        bool shouldRun();

    private:
        Mesh* _saloonMesh = nullptr;
        Mesh* _track1 = nullptr;


    private:
        GameState* _gameState = nullptr;
        Camera* _topDownGamePlayCamera = nullptr;
    };

} // adv

#endif //SIMPLE_KING_ADVENTURE_H
