//
// Created by mgrus on 21.03.2025.
//

#ifndef PLAYERBULLETPOOL_H
#define PLAYERBULLETPOOL_H



class PlayerBulletPool {
public:
    PlayerBulletPool(Scene* scene, Mesh* bulletMesh, Texture* bulletTexture, Shader* bulletShader);
    SceneNode* findFirstInactive();
    std::vector<SceneNode*> findAllActive();

private:
    std::vector<std::shared_ptr<SceneNode>> bulletNodes;

};



#endif //PLAYERBULLETPOOL_H
