//
// Created by mgrus on 21.03.2025.
//

#include "PlayerShooting.h"
#include <engine/game/input.h>
#include <engine/game/Scene.h>

PlayerShooting::PlayerShooting(const std::shared_ptr<SceneNode>& playerNode, const std::shared_ptr<PlayerBulletPool>& bulletPool) : _playerNode (playerNode), _bulletPool(bulletPool) {

}

void PlayerShooting::update() {
    static float lastShotInterval = 0.0f;
    lastShotInterval += ftSeconds;
    if (getControllerAxis(ControllerAxis::R_TRIGGER, 0) > 0.1 && lastShotInterval >= 0.1f) {
        lastShotInterval = 0.0f;
        auto bulletNode = _bulletPool->findFirstInactive();
        if (!bulletNode) {
            throw std::runtime_error("No bullet node found!!");
        }
        bulletNode->enable();

        glm::vec3 _shotOrigin = _playerNode->getLocation();
        glm::quat _shotDirection = _playerNode->getOrientation();
        glm::vec3 forward = _shotDirection * glm::vec3(0, 0, -1);
        bulletNode->setLocation(_shotOrigin + glm::vec3{0, 1.0f, 0} + forward * .5f);
        bulletNode->setOrientation(_shotDirection);
    }

    // render bullet stats code
#ifdef RENDER_BULLET_STATS
    int activeBullets = 0;
    for (auto b : playerBulletPool) {
        if (b->isActive()) {
            activeBullets++;
        }
    }

    sprintf_s(buf, 160, "# player bullets:%03d",
              activeBullets);
    flipUvs(false);
    font->renderText(buf, {2, scaled_height - 40, 0.88});
#endif

}
