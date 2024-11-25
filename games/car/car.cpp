//
// Created by mgrus on 07.06.2024.
//

#include "car.h"
#include "graphics.h"

namespace adv {

    Car::Car() {
    }

    bool Car::shouldStillRun() {
        return true;
    }

    void Car::update() {
    }

    void Car::render() {
        bindCamera(getGameplayCamera());
        bindTexture(_gameState->texTileMap);
        getGameplaySpritebatch()->render();


        scale({1, 1,1});
        foregroundColor({0.3, 0.6, 0.2, .1});
        location(glm::vec3{0, 0, 0});
        gridLines(100);
        drawGrid();

        lightingOn();


        // building
        bindTexture(nullptr);
        bindMesh(_saloonMesh);
        scale({.1, .1, .1});
        location(glm::vec3(0, 0, 0));
        rotation({0, 0, 0});
        drawMesh();

        // track
        bindTexture(nullptr);
        bindMesh(_track1);
        foregroundColor({0.1, 0.1, 0.1, 1});
        scale({1, 1, 1});
        location(glm::vec3(0, 0, 0));
        rotation({0, 0, 0});
        drawMesh();

        renderFPS();


    }



    void Car::init() {
            DefaultGame::init();

            _gameState = new GameState();
            _gameState->gameObjects = new std::vector<GameObject>(1000);
            _gameState->texTileMap = createTextureFromFile("../assets/binding/main_sprites.png", ColorFormat::RGBA);

            _saloonMesh = MeshImporter().importStaticMesh("../assets/saloon.glb");
            _track1 = MeshImporter().importStaticMesh("../assets/car/track1.glb");






    }

    std::string Car::getAssetFolder() {
        return "../assets/car";
    }

    Camera *Car::getGameplayCamera() {
        if (!_topDownGamePlayCamera) {
            _topDownGamePlayCamera = new Camera();
            _topDownGamePlayCamera->location = {0, 15, 5};
            _topDownGamePlayCamera->lookAtTarget = {0, 0, -5};
            _topDownGamePlayCamera->type = CameraType::Perspective;
        }

        return _topDownGamePlayCamera;
    }


} // adv

DefaultGame* getGame() {
    return new adv::Car();
}


void cargame::SceneNode::setModel(cargame::Model *model) {
    _model = model;
}

bool cargame::SceneNode::shouldRender() {
    return true;
}

cargame::Model *cargame::SceneNode::getModel() {
    return _model;
}

DirectX::XMFLOAT3 cargame::SceneNode::getPosition()  {
    return _position;
}

void cargame::SceneNode::setPosition(float x, float y, float z) {
    this->_position = {x, y, z};

}

ID3D11RasterizerState *cargame::SceneNode::getRasterizerState() {
    return _rs;
}

void cargame::SceneNode::setRasterizerState(ID3D11RasterizerState *rs) {
    _rs = rs;
}

void cargame::SceneNode::setScale(float x, float y, float z) {
    _scale = {x, y, z};
}

DirectX::XMFLOAT3 cargame::SceneNode::getScale() {
    return _scale;
}

DirectX::XMFLOAT2 cargame::SceneNode::getTextureScale() {
    return _textureScale;
}

void cargame::SceneNode::setTextureScale(float x, float y) {
    _textureScale = {x, y};
}

void cargame::SceneNode::setOrientation(DirectX::XMFLOAT3 eulers) {
    _orientation = eulers;
}

DirectX::XMFLOAT3 cargame::SceneNode::getOrientation() {
    return _orientation;
}

void cargame::Scene::render() {

    // Common projection and view matrix settings
    float ar = 1280.f / 720.0f;
    auto projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(50), ar, 0.1, 100);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    XMFLOAT3 camPos = {0, 30, -15};
    auto camFocusPoint = XMFLOAT3(0, 0, 1);
    auto camUp = XMFLOAT3(0, 1, 0);
    auto viewMatrix  = DirectX::XMMatrixLookAtLH(XMLoadFloat3(&camPos), XMLoadFloat3(&camFocusPoint), XMLoadFloat3(&camUp));
    viewMatrix = XMMatrixTranspose(viewMatrix);

    int w, h;
    GetWindowClientSize(gameData->hwnd, w, h);
    dx11::setViewport(0, 0, w, h);

    for (auto n : _nodes) {

        auto scale = n->getScale();
        auto scaleMat = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
        auto rotMat = DirectX::XMMatrixRotationY(n->getOrientation().y);
        auto pos = n->getPosition();
        auto posMat = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
        auto modelMat = posMat * scaleMat * rotMat;
        modelMat = XMMatrixTranspose(modelMat);

        auto model = n->getModel();

        dx11::renderMesh(model->matrixBuffer, modelMat, viewMatrix,
                         projectionMatrix, model->srv, model->samplerState, n->getRasterizerState(),
                         model->vertexBuffer, model->indexBuffer, model->uvBuffer, model->normalBuffer, model->indices.size(),
                         n->getTextureScale());

    }

}

void cargame::Scene::addNode(cargame::SceneNode *n) {
    _nodes.push_back(n);
}
