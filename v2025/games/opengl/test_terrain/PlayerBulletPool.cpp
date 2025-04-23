//
// Created by mgrus on 21.03.2025.
//

#include "PlayerBulletPool.h"

PlayerBulletPool::PlayerBulletPool(Scene *scene, Mesh *bulletMesh, Texture *bulletTexture, Shader* bulletShader) {

    // Create our bullet pool, a list of scene nodes which can be reused,
    // so we never respawn, but just use currently disabled ones.
    MeshDrawData playerBulletMeshData;
    playerBulletMeshData.texture = bulletTexture;
    playerBulletMeshData.mesh = bulletMesh;
    playerBulletMeshData.shader = bulletShader;
    playerBulletMeshData.castShadow = true;
    playerBulletMeshData.subroutineFragBind = "calculateSingleColor";
    playerBulletMeshData.shaderParameters.push_back({"emissionFactor", 6.0f});
    // playerBulletMeshData.onRender = [](MeshDrawData md) {
    //     glUseProgram(md.shader->handle);
    //     md.shader->setFloatValue(6.0f, "emissionFactor");
    // };
    //playerBulletMeshData->lit = false;
    for (int i = 0; i< 30; i++) {
        auto bulletNode = std::make_shared<SceneNode>("playerBullet_" + std::to_string(i));
        bulletNode->disable();
        bulletNode->initAsMeshNode(playerBulletMeshData);
        bulletNode->setScale({1, 1, 1});
        bulletNodes.push_back(bulletNode);
        scene->addNode(bulletNode);

        bulletNode->addComponent(std::make_unique<PlayerBulletComp>());
    }
}

SceneNode * PlayerBulletPool::findFirstInactive() {
    return Scene::findFirstInactive(bulletNodes);
}

std::vector<SceneNode *> PlayerBulletPool::findAllActive() {
    std::vector<SceneNode *> actives;
    for (auto n : bulletNodes) {
        if (n->isActive()) {
            actives.push_back(n.get());
        }
    }
    return actives;
}
