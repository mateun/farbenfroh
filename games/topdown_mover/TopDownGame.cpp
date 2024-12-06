//
// Created by mgrus on 17.11.2024.
//

#include "TopDownGame.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../src/engine/io/nljson.h"

extern ImFont* boldFont;

void TopDownGame::enemyLogicShooters() {

    for (auto e : shooterEnemies) {
        // Normally they move down.
        // When the strafe timer is over, they move to the left and right (strafe).

        glm::vec3 dir = {0, 0, 1};
        if (e->strafing) {
            dir = {e->strafeDir, 0, 0};
        }

        // e->strafeTimer += ftSeconds;
        // if (e->strafeTimer > e->strafeInterval) {
        //     e->strafeTimer = 0;
        //     e->strafing = !e->strafing;
        //     if (e->strafing) {
        //         e->strafeDir *= -1;
        //     }
        // }


        glm::ivec3 iloc = {(int)e->gameObject->location.x, 0, (int)e->gameObject->location.z};
        auto oldCellId = iloc.x + 20 * iloc.z;

        glm::vec3 plannedLocation = e->gameObject->location + glm::normalize(dir) * ftSeconds * .5f;
        glm::ivec3 ilocPlanned = {(int)plannedLocation.x, 0, (int)plannedLocation.z};
        auto cellId = ilocPlanned.x + 20 * ilocPlanned.z;
        // The target cell is unoccupied,
        // so we move there and set the old cell to empty.
        if (!enemyCells[cellId]) {
            e->gameObject->location = plannedLocation;
            enemyCells[oldCellId] = false;
            enemyCells[cellId] = true;
        } else {
            // If old and new cellid are identical we may still move fractional, otherwise we would be stuck forever:
            if (oldCellId == cellId) {
                e->gameObject->location = plannedLocation;
            }
        }
        if (e->wasHitThisFrame) {
            e->hitTimer += ftSeconds;
            if (e->hitTimer >= e->maxHitTime) {
                e->wasHitThisFrame = false;
                e->hitTimer = 0;
            }
        }

        e->shootTimer += ftSeconds;
        if (e->shootTimer > e->patternShootInterval) {
            e->bulletPattern->init();
            // for (auto eb: e->bulletPattern->getBullets()) {
            //     enemyBullets.push_back(eb);
            // }
            e->shootTimer = 0;
        }

        e->bulletPattern->update();
    }

}

void TopDownGame::enemyLogicSeekers() {
    for (auto e : seekerEnemies) {
        auto dir = player->location - e->gameObject->location;

        glm::ivec3 iloc = {(int)e->gameObject->location.x, 0, (int)e->gameObject->location.z};
        auto oldCellId = iloc.x + 20 * iloc.z;

        glm::vec3 plannedLocation = e->gameObject->location + glm::normalize(dir) * ftSeconds * 2.0f;
        glm::ivec3 ilocPlanned = {(int)plannedLocation.x, 0, (int)plannedLocation.z};
        auto cellId = ilocPlanned.x + 20 * ilocPlanned.z;
        // The target cell is unoccupied,
        // so we move there and set the old cell to empty.
        if (!enemyCells[cellId]) {
            e->gameObject->location = plannedLocation;
            enemyCells[oldCellId] = false;
            enemyCells[cellId] = true;
        } else {
            // If old and new cellid are identical we may still move fractional, otherwise we would be stuck forever:
            if (oldCellId == cellId) {
                e->gameObject->location = plannedLocation;
            }
        }

        if (e->wasHitThisFrame) {
            e->hitTimer += ftSeconds;
            if (e->hitTimer >= e->maxHitTime) {
                e->wasHitThisFrame = false;
                e->hitTimer = 0;
            }
        }

    }
}

void TopDownGame::updateInGameOver() {
    static bool musicPlaying = false;
    if (!musicPlaying) {
        playSound(getSoundByName("gameover"), true, -100);
        musicPlaying = true;
    }

    if (controllerButtonPressed(ControllerButtons::DPAD_DOWN, 0) || keyPressed(VK_DOWN)) {
        gameOverMenuIndex++;
        if (gameOverMenuIndex > 1) {
            gameOverMenuIndex = 1;
        }
    }

    if (controllerButtonPressed(ControllerButtons::DPAD_UP, 0) || keyPressed(VK_UP)) {
        gameOverMenuIndex--;
        if (gameOverMenuIndex < 0) {
            gameOverMenuIndex = 0;
        }
    }

    if (controllerButtonPressed(ControllerButtons::A_BUTTON, 0) ||keyPressed(VK_SPACE) || keyPressed(VK_RETURN)) {
        if (gameOverMenuIndex == 0) {
            stopSound(getSoundByName("gameover"));
            gameState = TopDownGameState::ReStart;
        } else if (gameOverMenuIndex == 1) {
            gameState = TopDownGameState::Exit;
        }
    }

}

void TopDownGame::updateInGameWon() {
    static bool musicPlaying = false;
    if (!musicPlaying) {
        playSound(getSoundByName("level"), true, -100);
        musicPlaying = true;
    }

    if (controllerButtonPressed(ControllerButtons::DPAD_DOWN, 0)) {
        gameWonMenuIndex++;
        if (gameWonMenuIndex > 1) {
            gameWonMenuIndex = 1;
        }
    }

    if (controllerButtonPressed(ControllerButtons::DPAD_UP, 0)) {
        gameWonMenuIndex--;
        if (gameWonMenuIndex < 0) {
            gameWonMenuIndex = 0;
        }
    }

    if (controllerButtonPressed(ControllerButtons::A_BUTTON, 0)) {
        if (gameWonMenuIndex == 0) {
            stopSound(getSoundByName("level"));
            gameState = TopDownGameState::ReStart;
        } else if (gameWonMenuIndex == 1) {
            gameState = TopDownGameState::Exit;
        }
    }
}

BulletPattern::BulletPattern(GameObject *parentGameObject, float speed) : parentGameObject(parentGameObject), speed(speed) {
}

bool BulletPattern::collides(AABB other) {
    for (auto b : bullets) {
        if (aabbsColliding(other, b->getUpdatedAABBByLocation())) {
            return true;
        }
    }

    return false;
}

std::vector<GameObject *> BulletPattern::getBullets() {
    return bullets;
}

SpreadLinePattern::SpreadLinePattern(GameObject *parentGameObject, float speed) : BulletPattern(parentGameObject, speed) {
}


void SpreadLinePattern::update() {
    std::vector<GameObject*> bulletRemovals;
    for (auto b : bullets) {
        b->location += b->forward * ftSeconds * speed;
        b->timeAlive += ftSeconds;
        if (b->timeAlive > 40) {
            bulletRemovals.push_back(b);
        }
    }
    for (auto b: bulletRemovals) {
        bullets.erase(std::remove(bullets.begin(), bullets.end(), b), bullets.end());
    }
}

void SpreadLinePattern::render() {
    std::vector<glm::mat4> matrices;
    std::vector<glm::vec4> colors;
    for (auto w: bullets) {
        glm::mat4 mattrans = glm::translate(glm::mat4(1), w->location);
        glm::mat4 matscale = glm::scale(glm::mat4(1), w->scale);
        glm::mat4 matworld = mattrans * matscale;
        matrices.push_back(matworld);
        colors.push_back({1, 1, 0, 1});
    }
    setInstanceMatrices(matrices);
    setInstanceColors(colors);
    drawMeshInstanced(matrices.size());
}

void SpreadLinePattern::init() {
    float shootAngle = -45;
    int numBullets = 15;
    for (int i = 0; i < numBullets; i++) {
        auto eb = new GameObject();
        eb->location = parentGameObject->location;
        eb->scale = {0.2, 0.2, 0.2};
        eb->aabb = {-0.1, 0, 0.1, 0.1, 0, -.1};
        eb->forward = glm::normalize(glm::rotate(glm::mat4(1), glm::radians(shootAngle), {0, 1, 0}) * glm::vec4(0, 0, 1, 0));
        shootAngle += (45.0f / numBullets) + 5;
        bullets.push_back(eb);
    }

}

Stage::Stage(const std::string &id) : id(id) {
}

Wave::Wave(const std::string& id) : id(id) {

}

void Stage::addWave(Wave *wave) {
    waves.push_back(wave);
}

std::vector<Wave *> Stage::getWaves() {
    return waves;
}

void StageSequence::addStage(Stage *stage) {
    stages.push_back(stage);
}

std::vector<Stage *> StageSequence::getStages() {
    return stages;
}

void TopDownGame::update() {
    static bool bgMusicPlaying = false;

    // Hard pause, no more updates running.
    {
        if (keyPressed('P')) {
            paused = !paused;
        }
        if (paused) return;
    }


    if (controllerButtonPressed(ControllerButtons::MENU, 0) || keyPressed(VK_ESCAPE)) {
        if (gameState != TopDownGameState::Settings) {
            prevGameState = gameState;
        }
        gameState = TopDownGameState::Settings;
    }

    if (gameState == TopDownGameState::MainMenu) {
        updateInMainMenu();
        return;
    }

    if (gameState == TopDownGameState::Settings) {
        updateInSettings();
        return;
    }

    if (gameState == TopDownGameState::GameOver) {
        updateInGameOver();
        return;
    }

    if (gameState == TopDownGameState::GameWon) {
        updateInGameWon();
        return;
    }
    if (gameState == TopDownGameState::DisplayStageWave) {
        updateInDisplayStageWave();
        return;
    }

    if (gameState == TopDownGameState::ReStart) {
        lives = 3;
        for (auto sh : shooterEnemies) {
            delete(sh);
        }
        shooterEnemies.clear();
        for (auto se : seekerEnemies) {
            delete(se);
        }
        seekerEnemies.clear();
        for (auto b : bullets) {
            delete(b);
        }
        bullets.clear();

        player->location = playerStartPosition;
        initEnemies();
        bgMusicPlaying = false;
        gameState = TopDownGameState::InGame;
    }

    if (gameState == TopDownGameState::NextStage) {
        lives = 3;
        for (auto sh : shooterEnemies) {
            delete(sh);
        }
        shooterEnemies.clear();
        for (auto se : seekerEnemies) {
            delete(se);
        }
        seekerEnemies.clear();
        for (auto b : bullets) {
            delete(b);
        }
        bullets.clear();

        player->location = playerStartPosition;
        initEnemies();
        bgMusicPlaying = false;
        gameState = TopDownGameState::InGame;
    }

    if (gameState == TopDownGameState::Editor) {
        updateInEditor();
        return;
    }

    // If we are in no "special" state, we are just in game and so we continue here onwards.

    // Play some bg music
    if (!bgMusicPlaying) {
        playSound(getSoundByName("arcade_loop"), true);
        bgMusicPlaying = true;
    }

    // Check for win condition of this stage/wave:
    if (seekerEnemies.empty() && shooterEnemies.empty()) {
        wave++;
        if (wave > 2) {
            stage++;
            wave = 0;
        }

        if (stage == 2) {
            gameState = TopDownGameState::GameWon;
            stopSound(getSoundByName("arcade_loop"));
            return;
        }
        gameState = TopDownGameState::DisplayStageWave;
        return;
    }

    playerAnimPlayer->update();
    for (auto pe : particleEffects) {
        updateParticleEffect(pe.second);
    }

    // Player movement
    {
        float speed = 5;
        float fwd = 0;
        float strafe = 0;

        if (isKeyDown('W') || getControllerAxis(ControllerAxis::LSTICK_Y, 0) > 0.4) {
            fwd -= 1;
        }
        if (isKeyDown('S') || getControllerAxis(ControllerAxis::LSTICK_Y, 0) < -0.4) {
            fwd += 1;
        }

        if (isKeyDown('A') || getControllerAxis(ControllerAxis::LSTICK_X, 0) < -0.4) {
            strafe -= 1;
        }
        if (isKeyDown('D') || getControllerAxis(ControllerAxis::LSTICK_X, 0) > 0.4) {
            strafe += 1;
        }

        glm::vec2 shootDir = {0, 0};
        // For mouse controls:
        // Get point in world where mouse cursor hits the ground plane.
        // Then this is the shoot target point, which decides the shoot direction.
        auto ray = createRayFromCameraAndMouse(getGameplayCamera());
        glm::vec3 hitPoint;
        if (rayIntersectsPlane(ray, {0, 1, 0}, {1, 0, 0}, &hitPoint)) {
            auto dir = glm::normalize(hitPoint - player->location);
            shootDir = {dir.x, -dir.z};
        }

        // Gamepad shoot controls:
        // Here we just directly use horizontal and vertical input.
        if (getControllerAxis(ControllerAxis::RSTICK_Y, 0) > 0.2) {
            shootDir.y = getControllerAxis(ControllerAxis::RSTICK_Y, 0);
        }
        if (getControllerAxis(ControllerAxis::RSTICK_Y, 0) < -0.2) {
            shootDir.y = getControllerAxis(ControllerAxis::RSTICK_Y, 0);
        }

        if (getControllerAxis(ControllerAxis::RSTICK_X, 0) > 0.2) {
            shootDir.x = getControllerAxis(ControllerAxis::RSTICK_X, 0);
        }
        if (getControllerAxis(ControllerAxis::RSTICK_X, 0) < -0.2) {
            shootDir.x = getControllerAxis(ControllerAxis::RSTICK_X, 0);
        }

        if (getControllerAxis(ControllerAxis::R_TRIGGER, 0) > 0.5 || lbuttonDown) {
            shootTriggerPressed = true;
        } else {
            shootTriggerPressed = false;
        }


        glm::vec3 velocity = glm::vec3{strafe, 0, fwd} * ftSeconds * speed;
        //player->rotation += glm::vec3{0, yaw, 0} * ftSeconds * 450.0f;
        //player->forward = glm::normalize(glm::rotate(glm::mat4(1), glm::radians(player->rotation.y), {0, 1, 0}) * glm::vec4{0, 0, 1, 0});

        if (length(shootDir) > 0) {
            player->forward = glm::normalize(glm::vec3(shootDir.x, 0, -shootDir.y));
            float yawAngle = glm::atan(shootDir.x, -shootDir.y);
            player->rotation = {0, glm::degrees(yawAngle), 0};
        }
        shootGuide->rotation = player->rotation;

        auto projectedLocation = player->location + velocity;
        auto updatedAABB = getMovedABBByLocation(player->aabb, player->location + velocity);

        // Now check for the separate collision axis and implement sliding
        bool anyCollision = false;

        int numCollisions = 0;
        for (auto w : walls) {
            auto collides = aabbsColliding(updatedAABB, w->aabb);
            if (collides) {
                numCollisions += 1;
            }

        }

        if (numCollisions > 1) {
            printf("double coll!\n");
        }

        for (auto w : walls) {
            auto collides = aabbsColliding(updatedAABB, w->aabb);
            if (collides) {
                // implement sliding
                // We need the correct normal of the poly we have been colliding with.
                Ray ray;
                ray.origin = player->location;
                ray.direction = glm::normalize(velocity);
                ray.maxLength = 50;
                glm::vec3 normal = w->collideWithRay(ray);
                glm::vec3 invertedNormal = -normal;
                invertedNormal *= glm::length(velocity);
                glm::vec3 slideDirection = velocity - invertedNormal;
                if (numCollisions < 2) {
                    player->location += slideDirection;
                }
                anyCollision = true;
            }
        }

        if (!anyCollision) {
            player->location = projectedLocation;
        }
        getGameplayCamera()->updateFollow();
        shootGuide->location = player->location + player->forward * -0.75f;

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Player shooting
        // Check for shooting key press with 1 second cooldown.
        static bool shootAllowed = false;
        static float triggerTimer = 0;
        triggerTimer += ftSeconds;
        if (triggerTimer > 0.1) {
            shootAllowed = true;
            triggerTimer = 0;
        }

        if (shootTriggerPressed > 0 && shootAllowed) {
            auto bullet = new GameObject();
            bullet->location = player->location + glm::normalize(player->forward) * 0.5f ;
            bullet->scale = {.2, .2,.3 };
            bullet->aabb = {-0.1, 0, 0.1, 0.1, 0, -.1};
            bullet->forward = player->forward;
            bullet->timeAlive = 0;
            bullets.push_back(bullet);
            shootAllowed = false;
            triggerTimer = 0;
            playSound(getSoundByName("laser2"), false);
        }

        std::vector<GameObject*> outlived;

        // Update all bullets
        int index = 0;
        for (auto b : bullets) {
            b->location += b->forward * ftSeconds * 20.0f;
            b->timeAlive += ftSeconds;
            if (b->timeAlive > 3) {
                outlived.push_back(b);
            }
            index++;
        }

        for (auto i : outlived) {
            bullets.erase(std::remove(bullets.begin(), bullets.end(), i), bullets.end());
        }
        // End player shooting and bullets logic
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }




    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Enemy logic - moving and shooting.
    // Collision with player is handled later here.
    {
        enemyLogicSeekers();
        enemyLogicShooters();

    }

    // End enemy logic
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Enemy <> player-bullet collisions
    std::vector<Enemy*> deadEnemies;
    std::vector<GameObject*> bulletsToRemove;
    bool hitAnEnemy = false;
    for (auto b : bullets) {

        for (auto e : seekerEnemies) {
            if (aabbsColliding(b->getUpdatedAABBByLocation(), e->gameObject->getUpdatedAABBByLocation())) {
                e->setHit();
                if (e->hp <= 0) {
                    deadEnemies.push_back(e);
                    playSound(getSoundByName("explosion"), false);
                    resetParticleEffect(particleEffects["explosion"]);
                    particleEffects["explosion"]->location = e->gameObject->location;
                }
                e->wasHitThisFrame = true;
                hitAnEnemy = true;
                bulletsToRemove.push_back(b);

                // TODO thinking of how to enhance enemy functions without baking everything directly into the gameObject?

            }
        }
        for (auto e : shooterEnemies) {
            if (aabbsColliding(b->getUpdatedAABBByLocation(), e->gameObject->getUpdatedAABBByLocation())) {
                e->setHit();
                if (e->hp <= 0) {
                    deadEnemies.push_back(e);
                    playSound(getSoundByName("explosion"), false);

                    resetParticleEffect(particleEffects["explosion"]);
                    particleEffects["explosion"]->location = e->gameObject->location;
                }
                e->wasHitThisFrame = true;
                hitAnEnemy = true;
                bulletsToRemove.push_back(b);
                // TODO thinking of how to enhance enemy functions without baking everything directly into the gameObject?
            }
        }
    }

    if (hitAnEnemy) {
        playSound(getSoundByName("enemy_hit"), false);
    }

    for (auto e : deadEnemies) {
        seekerEnemies.erase(std::remove(seekerEnemies.begin(), seekerEnemies.end(), e), seekerEnemies.end());
        shooterEnemies.erase(std::remove(shooterEnemies.begin(), shooterEnemies.end(), e), shooterEnemies.end());
        enemyCells[e->gameObject->location.x + e->gameObject->location.z * mapTilesHorizontal];
        delete(e);
    }

    for (auto b : bulletsToRemove) {
        bullets.erase(std::remove(bullets.begin(), bullets.end(), b), bullets.end());
        delete(b);
    }
    // End enemy <> player-bullet collisions

    // Player <> enemy-bullet collision
    if (!playerBulletImmune) {
        // Make a smaller hitbox for our bullet collisions to make it a bit easier on the player.
        auto shrunkAABB = player->aabb;
        shrunkAABB.scaleBy(0.2);
        auto playerAABBShrunk = player->getUpdatedOtherAABBByLocation(shrunkAABB);

        for (auto e : shooterEnemies) {
            if (e->bulletPattern->collides(playerAABBShrunk)) {
                playSound(getSoundByName("player_hit"), false, -50);
                lives--;
                if (lives <= 0) {
                    gameState = TopDownGameState::GameOver;
                    stopSound(getSoundByName("arcade_loop"));
                }
                playerHitByBulletThisFrame = true;
                playerBulletImmune = true;
                break;
            }
        }
    }

    if (playerHitByBulletThisFrame) {
        playerHitByBulletTimer += ftSeconds;
        if (playerHitByBulletTimer >= playerHitByBulletMaxHitTime) {
            playerHitByBulletThisFrame = false;
            playerHitByBulletTimer = 0;
        }
    }

    if (playerBulletImmune) {
        playerBulletImmuneTimer += ftSeconds;
        if (playerBulletImmuneTimer > playerBulletImmuneMax) {
            playerBulletImmuneTimer = 0;
            playerBulletImmune = false;
        }
    }


    // Player enemy collision
    if (!playerImmune) {
        for (auto e : seekerEnemies) {
            if (aabbsColliding(player->getUpdatedAABBByLocation(), e->gameObject->getUpdatedAABBByLocation())) {
                playSound(getSoundByName("player_hit"), false, -100);

                lives--;
                if (lives <= 0) {
                    gameState = TopDownGameState::GameOver;
                    stopSound(getSoundByName("arcade_loop"));
                }
                playerHitThisFrame = true;
                playerImmune = true;
                break;

            }
        }
    }

    if (playerHitThisFrame) {
        playerHitTimer += ftSeconds;
        if (playerHitTimer >= playerMaxHitTime) {
            playerHitThisFrame = false;
            playerHitTimer = 0;
        }
    }

    if (playerImmune) {
        playerImmuneTimer += ftSeconds;
        if (playerImmuneTimer > playerImmuneMax) {
            playerImmuneTimer = 0;
            playerImmune = false;
        }
    }

}

void TopDownGame::initEnemies() {

    // Lets try out the json file loading:
    // std::ifstream f("stages.json");
    // nlohmann::json data = nlohmann::json::parse(f);
    // auto stages = data["stages"];
    // for (auto stage : stages) {
    //     std::string name = stage["name"];
    //     printf("stage name: %s\n", name.c_str());
    // }

    // if not json file is here we use some hardcoded stuff, but this is not good!!
    // TODO exit here?!

    if (stage == 0) {
        int numSeekers = 4;
        int numShooters = 3;
        if (wave == 0) {
            numSeekers = 4;

        }
        if (wave == 1) {
            numSeekers = 5;
            numShooters = 4;
        }
        if (wave == 2) {
            numSeekers = 6;
            numShooters = 5;
        }

        // First the seekers
        for (int i = 0; i < numSeekers; i++) {
            auto seekerGO = new GameObject();
            const glm::vec3 location = {-numSeekers / 2 + (5 * i) , 0, -8};
            seekerGO->location = location;
            seekerGO->scale = {1,1, 1};
            seekerGO->aabb = {-0.5, 0, 0.5, 0.5, 3, -.5};

            auto seekerEnemy = new Enemy(nullptr);
            seekerEnemy->gameObject = seekerGO;
            seekerEnemies.push_back(seekerEnemy);
            enemyCells[location.x + location.z * mapTilesHorizontal] = true;

        }

        // Shooter enemies setup

        for (int i = 0; i < numShooters; i++) {
            auto go = new GameObject();
            go->location = { -numShooters/2 - 2 + (i*4), 0, -9};
            go->scale = { 0.4, 1.2, 0.6};
            go->aabb = {-0.5, 0, 0.5, 0.5, 2, -0.5};
            auto se = new Enemy(new SpreadLinePattern(go));
            se->gameObject = go;
            shooterEnemies.push_back(se);
            enemyCells[-numShooters/2 - 2 + (i*4) + -9 * mapTilesHorizontal];
        }
    }

    if (stage == 1) {
        int numSeekers = 4;
        int numShooters = 3;
        if (wave == 0) {
            numSeekers = 4;

        }
        if (wave == 1) {
            numSeekers = 5;
            numShooters = 4;
        }
        if (wave == 2) {
            numSeekers = 6;
            numShooters = 5;
        }

        // First the seekers
        for (int i = 0; i < numSeekers; i++) {
            auto seekerGO = new GameObject();
            const glm::vec3 location = {-numSeekers / 2 + (5 * i) , 0, -8};
            seekerGO->location = location;
            seekerGO->scale = {1,1, 1};
            seekerGO->aabb = {-0.5, 0, 0.5, 0.5, 3, -.5};

            auto seekerEnemy = new Enemy(nullptr);
            seekerEnemy->gameObject = seekerGO;
            seekerEnemies.push_back(seekerEnemy);
            enemyCells[location.x + location.z * mapTilesHorizontal] = true;

        }

        // Shooter enemies setup
        for (int i = 0; i < numShooters; i++) {
            auto go = new GameObject();
            go->location = { -numShooters/2 - 2 + (i*4), 0, -9};
            go->scale = { 0.4, 1.2, 0.6};
            go->aabb = {-0.5, 0, 0.5, 0.5, 2, -0.5};
            auto se = new Enemy(new SpreadLinePattern(go));
            se->gameObject = go;
            shooterEnemies.push_back(se);
            enemyCells[-numShooters/2 - 2 + (i*4) + -9 * mapTilesHorizontal];
        }
    }

}

void TopDownGame::init() {

    // Some math testing
    // So the dot product shows me how much one vector points in the direction of another one.
    // 1. example: parallel
    glm::vec3 v1 = {1, 0, 0};
    glm::vec3 v2 = {3, 0, 0};
    float result = glm::dot(v1, glm::normalize(v2));

    // 2. example: normal to each other
    v1 = {1, 0,0 };
    v2 = {0, 1, 0};
    result = glm::dot(v1, v2);

    // 3. example: opposite direction
    v1 = {1, 0,0 };
    v2 = {-1, 0.2, 0};
    result = glm::dot(v1, v2);

    v1 = {1, 0,0 };
    v2 = {-0.01, 0.2, 0};
    result = glm::dot(v1, glm::normalize(v2));

    v1 = {10, 10,0 };
    v2 = {100, 40, 0};
    result = glm::dot(v1, glm::normalize(v2));
    glm::vec3 projectionPoint = glm::normalize(v2) * result;

    // test point in tri function
    glm::vec3 point = {0, 0, 0};
    glm::vec3 A = {-2, -1, 0};
    glm::vec3 B = {-1,2, 0};
    glm::vec3 C = {3, 1, 0};
    auto inTri = pointInTri(point, A, B, C);
    assert(inTri == true);
    point = {-2, 2, 0};
    inTri = pointInTri(point, A, B, C);
    assert(inTri == false);

    point = {-1, -0, 0};
    inTri = pointInTri(point, A, B, C);
    assert(inTri == true);

    // end math testing here.
    DefaultGame::init();

    player = new GameObject();
    player->mesh = getMeshByName("king_cartoon");
    player->name = "player";
    playerAnimPlayer = new AnimationPlayer(player->mesh->animations[0], player->mesh);
    playerAnimPlayer->play(true);
    player->location = playerStartPosition;
    player->aabb = {-0.5, 0, 0.5, 0.5, 2, -0.5 };

    shootGuide = new GameObject();
    shootGuide->mesh = getMeshByName("shootguide");

    // Enemies setup
    initEnemies();

    // Generate walls for the given mapsize.
    // We always have 4 walls.
    auto w1 = new GameObject();
    w1->mesh = getMeshByName("unit_cube");
    w1->location = {-mapTilesHorizontal / 2, 0, 0};
    w1->scale = {1, 3, mapTilesVertical };
    w1->collisionMeshScale = {1, 3, mapTilesVertical};
    w1->aabb = {-0.5, 0, ((float)mapTilesVertical/2), 0.5, 3, (float)-mapTilesVertical/2};
    w1->aabb = w1->getUpdatedAABBByLocation();

    auto w2 = new GameObject();
    w2->mesh = getMeshByName("unit_cube");
    w2->location = {mapTilesHorizontal/2, 0, 0};
    w2->scale = {1, 3, mapTilesVertical};
    w2->collisionMeshScale = {1, 3, mapTilesVertical + 2};
    w2->aabb = {-0.5, 0, (float)mapTilesVertical/2, 0.5, 3, -(float)mapTilesVertical/2};
    w2->aabb = w2->getUpdatedAABBByLocation();

    auto w3 = new GameObject();
    w3->mesh = getMeshByName("unit_cube");
    w3->location = {0, 0, -mapTilesVertical/2 - 0.5};
    w3->scale = {mapTilesHorizontal+1, 3, 1};
    w3->collisionMeshScale = {mapTilesHorizontal+1, 3, 1};
    w3->aabb = {(float)-mapTilesHorizontal/2+1, 0, 0.5, (float)mapTilesHorizontal/2, 3, -0.5};
    w3->aabb = w3->getUpdatedAABBByLocation();

    auto w4 = new GameObject();
    w4->mesh = getMeshByName("unit_cube");
    w4->location = {0, 0, mapTilesVertical/2+0.5};
    w4->scale = {mapTilesHorizontal+1, 3, 1};
    w4->collisionMeshScale = {mapTilesHorizontal+1, 3, 1};
    w4->aabb = {-((float)mapTilesHorizontal/2)+1, 0, 0.5, ((float)mapTilesHorizontal/2), 3, -.5};
    w4->aabb = w4->getUpdatedAABBByLocation();

    walls.push_back(w1);
    walls.push_back(w2);
    walls.push_back(w3);
    walls.push_back(w4);

    getGameplayCamera()->follow(player, {0, 19, 6});

    hudFont = new FBFont("../assets/font.bmp");

    gameState = TopDownGameState::MainMenu;


    // Init particle effects:
    ParticleEffect* explosionEffect = new ParticleEffect();
    explosionEffect->location = {-3, 0, -2};
    explosionEffect->initialSpeed = 0.09;
    explosionEffect->spawnLag = 0.5;
    explosionEffect->number = 18;
    explosionEffect->gravityValue = -1.45;
    explosionEffect->maxLifeInSeconds = 1.7;
    explosionEffect->oneShot = true;
    explosionEffect->mesh = getMeshByName("unit_cube");
    explosionEffect->texture = getTextureByName("smoke_white");
    explosionEffect->rotation = glm::vec3{-45, 0, 0};
    explosionEffect->done = true;
    particleEffects["explosion"] = explosionEffect;


}

void TopDownGame::render() {

    if (gameState == TopDownGameState::MainMenu) {
        renderInMainMenu();
        return;
    }
    if (gameState == TopDownGameState::Settings) {
        renderInSettings();
        return;
    }
    if (gameState == TopDownGameState::GameOver) {
        renderInGameOver();
        return;
    }
    if (gameState == TopDownGameState::GameWon) {
        renderInGameWon();
        return;
    }
    if (gameState == TopDownGameState::DisplayStageWave) {
        renderInDisplayStageWave();
        return;
    }

    if (gameState == TopDownGameState::Editor) {
        renderInEditor();
        return;
    }

    DefaultGame::render();

//    bindMesh(getMeshByName("saloon"));
//    foregroundColor({.8, .8, .8, 1});
//    location({-8, 1, -3});
//    scale({0.5, 0.5, 0.5});
//    drawMesh();

    // Draw the walls.
    {
        foregroundColor({.8, .8, .8, 1});
        for (auto w: walls) {
            scale(w->scale);
            location(w->location);
            bindMesh(w->mesh);
            drawMesh();
        }
    }

    // Draw the bullets
    {
        deferredStart();
        foregroundColor({1, .2, .2, 1});
        bindMesh(getMeshByName("unit_cube"));
        //bindTexture(getTextureByName("king_diffuse"));
        shadowOff();
        bindTexture(nullptr);
        std::vector<glm::mat4> matrices;
        std::vector<glm::vec4> colors;
        for (auto w: bullets) {
            glm::mat4 mattrans = glm::translate(glm::mat4(1), w->location);
            glm::mat4 matscale = glm::scale(glm::mat4(1), w->scale);
            glm::mat4 matworld = mattrans * matscale;
            matrices.push_back(matworld);
            colors.push_back({1, 0, 0, 1});
        }
        setInstanceMatrices(matrices);
        setInstanceColors(colors);
        drawMeshInstanced(matrices.size());


        // Enemy bullets
        for (auto e : shooterEnemies) {
            e->bulletPattern->render();
        }

        deferredEnd();
    }




    // Draw the player
    if (lives > 0) {
        bindMesh(player->mesh);
        bindTexture(getTextureByName("king_diffuse"));
        if (playerHitThisFrame) {
            tint(glm::vec4(1, 0, 0, 1));
        } else {
            tint(glm::vec4(1, 1, 1, 1));
        }
        scale({1, 1, 1});
        location(player->location);
        rotation(player->rotation);
        scale({.01, .01, .01});
        //rotation({0, 180, 0});
        flipUvs(true);
        setSkinnedDraw(true);
        drawMesh();
        setSkinnedDraw(false);

        // Draw the shoot guide
        scale({0.5, 1, 4});
        bindMesh(shootGuide->mesh);
        rotation(shootGuide->rotation);
        location(shootGuide->location);

        bindTexture(getTextureByName("shootguide_diffuse"));
        drawMesh();
        rotation({0, 0, 0});
    }

    // Draw the enemies
    deferredStart();
    bindMesh(getMeshByName("spiderbot"));
    bindTexture(getTextureByName("king_diffuse"));
    glm::vec4 hitColor = {1, 1, 0, 1};
    {
        std::vector<glm::mat4> matrices;
        std::vector<glm::vec4> tints;
        for (auto e : seekerEnemies) {
            glm::mat4 mattrans = glm::translate(glm::mat4(1), e->gameObject->location);
            glm::mat4 matscale = glm::scale(glm::mat4(1), e->gameObject->scale);
            glm::mat4 matworld = mattrans * matscale;
            matrices.push_back(matworld);
            if (e->wasHitThisFrame) {
                tints.push_back(hitColor);
            } else {
                tints.push_back({1, 1,1, 1});
            }
        }
        setInstanceTints(tints);
        setInstanceMatrices(matrices);
        drawMeshInstanced(matrices.size());
    }

    {
        std::vector<glm::mat4> matrices;
        std::vector<glm::vec4> tints;
        for (auto e : shooterEnemies) {
            glm::mat4 mattrans = glm::translate(glm::mat4(1), e->gameObject->location);
            glm::mat4 matscale = glm::scale(glm::mat4(1), e->gameObject->scale);
            glm::mat4 matworld = mattrans * matscale;
            matrices.push_back(matworld);

            if (e->wasHitThisFrame) {
                tints.push_back(hitColor);
            } else {
                tints.push_back({1, 1,1, 1});
            }
        }
        setInstanceTints(tints);
        setInstanceMatrices(matrices);
        drawMeshInstanced(matrices.size());

    }
    deferredEnd();

    // Draw active particle effects
    for (auto pe : particleEffects) {
        if (!pe.second->done) {
            bindParticleEffect(pe.second);
            drawParticleEffect();
        }
    }

    flipUvs(false);

    bindCamera(getUICamera());
    foregroundColor({.99, .99, 0.99, 1});
    lightingOff();
    //hudFont->renderText("LIVES ", {scaled_width/2 + 256 * 4, scaled_height/2, -1});
    glDefaultObjects->currentRenderState->textScale = {2, 2};
    hudFont->renderText("LIVES: " + std::to_string(lives), {540, scaled_height - 70, -1});
    glDefaultObjects->currentRenderState->textScale = {1, 1};

    DefaultGame::renderFPS();
}

bool TopDownGame::shouldStillRun() {
    return gameState != TopDownGameState::Exit;
}

std::vector<std::string> TopDownGame::getAssetFolder() {
    return { "../games/king1024/assets", "../assets/sound"};
}

bool TopDownGame::shouldAutoImportAssets() {
    return true;
}

void TopDownGame::renderInGameOver() {
    bindCamera(getUICamera());
    foregroundColor({.99, .09, 0.09, 1});
    lightingOff();
    glDefaultObjects->currentRenderState->textScale = {7, 7};
    hudFont->renderText("GAME OVER! ", {50, scaled_height - 450, -9});
    glDefaultObjects->currentRenderState->textScale = {4, 4};
    hudFont->renderText("SCORE: " + std::to_string(score), {50, scaled_height - 490, -8});


    glDefaultObjects->currentRenderState->textScale = {2, 2};

    hudFont->renderText("Back to Game", {100, scaled_height - 570, -7});
    hudFont->renderText("Exit", {100, scaled_height - 600, -6});

    // Cursor
    int cursorY = scaled_height - (570 + (gameOverMenuIndex * 30));
    hudFont->renderText("o", {50, cursorY, -3});


    glDefaultObjects->currentRenderState->textScale = {1, 1};
}

void TopDownGame::renderInGameWon() {
    bindCamera(getUICamera());
    foregroundColor({.99, .09, 0.09, 1});
    lightingOff();
    glDefaultObjects->currentRenderState->textScale = {7, 7};
    hudFont->renderText("GAME WON! ", {50, scaled_height - 450, -9});
    glDefaultObjects->currentRenderState->textScale = {4, 4};
    hudFont->renderText("SCORE: " + std::to_string(score), {50, scaled_height - 490, -8});


    glDefaultObjects->currentRenderState->textScale = {2, 2};

    hudFont->renderText("Back to Game", {100, scaled_height - 570, -7});
    hudFont->renderText("Exit", {100, scaled_height - 600, -6});

    // Cursor
    int cursorY = scaled_height - (570 + (gameWonMenuIndex * 30));
    hudFont->renderText("o", {50, cursorY, -3});


    glDefaultObjects->currentRenderState->textScale = {1, 1};
}

void TopDownGame::renderInMainMenu() {
    auto err = glGetError();
    bindCamera(getUICamera());
    foregroundColor({.99, .09, 0.09, 1});
    lightingOff();
    glDefaultObjects->currentRenderState->textScale = {4, 4};
    hudFont->renderText("KING ARENA", {100, scaled_height - 450, -10});
    err = glGetError();

    glDefaultObjects->currentRenderState->textScale = {2, 2};

    hudFont->renderText("Play", {100, scaled_height - 480, -9});
    hudFont->renderText("Settings", {100, scaled_height - 510, -8});
    hudFont->renderText("Exit", {100, scaled_height - 540, -7});

    // Cursor
    int cursorY = scaled_height - (480 + (mainMenuIndex * 30));
    hudFont->renderText("o", {50, cursorY, -3});

    glDefaultObjects->currentRenderState->textScale = {1, 1};

}

void TopDownGame::updateInMainMenu() {
    static bool musicPlaying = false;
    if (!musicPlaying) {
        musicPlaying = true;
        auto mainMenuMusic = getSoundByName("final_frontier_bg");
        playSound(mainMenuMusic, true);
        setCurrentMusic(mainMenuMusic);
    }

    bool selectedItem =false;
    if (controllerButtonPressed(ControllerButtons::DPAD_DOWN, 0) || keyPressed('S') || keyPressed(VK_DOWN)) {
        mainMenuIndex++;
        if (mainMenuIndex > 2) {
            mainMenuIndex = 2;
        }
        selectedItem = true;
    }

    if (controllerButtonPressed(ControllerButtons::DPAD_UP, 0) || keyPressed('W') || keyPressed(VK_UP)) {
        mainMenuIndex--;
        if (mainMenuIndex < 0) {
            mainMenuIndex = 0;
        }
        selectedItem = true;
    }

    if (controllerButtonPressed(ControllerButtons::A_BUTTON, 0) || keyPressed(VK_SPACE) || keyPressed(VK_RETURN)) {
        if (mainMenuIndex == 0) {
            gameState = TopDownGameState::DisplayStageWave;
            //gameState = TopDownGameState::InGame;
            stopSound(getSoundByName("final_frontier_bg"));
        }
        else if (mainMenuIndex == 1) {
            prevGameState = TopDownGameState::MainMenu;
            gameState = TopDownGameState::Settings;
        }
        else if (mainMenuIndex == 2) {
            gameState = TopDownGameState::Exit;
        }
    }
    if (selectedItem) {
        playSound(getSoundByName("menu1"), false);
    }
}

void TopDownGame::updateInSettings() {

    if (controllerButtonPressed(ControllerButtons::DPAD_DOWN, 0) || keyPressed(VK_DOWN)) {
        settingsMenuIndex++;
        if (settingsMenuIndex > 5) {
            settingsMenuIndex = 5;
        }
    }

    if (controllerButtonPressed(ControllerButtons::DPAD_UP, 0) || keyPressed(VK_UP)) {
        settingsMenuIndex--;
        if (settingsMenuIndex < 0) {
            settingsMenuIndex = 0;
        }
    }

    if (controllerButtonPressed(ControllerButtons::A_BUTTON, 0) || keyPressed(VK_SPACE) || keyPressed(VK_RETURN)) {
        if (settingsMenuIndex == 3) {
            stopSound(getCurrentlyPlayingMusic());
            gameState = TopDownGameState::Editor;
        } else if (settingsMenuIndex == 4) {
            gameState = prevGameState;
        } else if (settingsMenuIndex == 5) {
            gameState = TopDownGameState::Exit;
        }
    }

    if (controllerButtonPressed(ControllerButtons::B_BUTTON, 0) || keyPressed(VK_ESCAPE)) {
        if (prevGameState == TopDownGameState::InGame) {
            gameState = TopDownGameState::InGame;
        } else if (prevGameState == TopDownGameState::MainMenu) {
            gameState = TopDownGameState::MainMenu;
        }

    }

}

void TopDownGame::updateInEditor() {
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        currentEditorstageSequence = new StageSequence();
        auto s0 = new Stage("s0");
        auto s1 = new Stage("s1");

        currentEditorstageSequence->addStage(s0);
        currentEditorstageSequence->addStage(s1);

        auto s0w0 = new Wave("s0w0");
        auto s0w1 = new Wave("s0w1");
        auto s1w0 = new Wave("s1w0");
        auto s1w1 = new Wave("s1w1");
        s0->addWave(s0w0);
        s0->addWave(s0w1);
        s1->addWave(s1w0);
        s1->addWave(s1w1);

        currentEditorStage = s0;
        currentEditorWave = s0w0;

    }


}

void TopDownGame::updateInDisplayStageWave() {

    static float stageWaveTimer = 0;
    stageWaveTimer += ftSeconds;

    if (stageWaveTimer > .5f) {
        stageWaveTimerCounter--;
        stageWaveTimer = 0;
    }

    if (stageWaveTimerCounter == 0) {
        stageWaveTimerCounter = 3;
        stageWaveTimer = 0;
        if (wave == 0 && stage == 0) {
            gameState = TopDownGameState::InGame;
        } else {
            gameState = TopDownGameState::NextStage;
        }


    }

}

void TopDownGame::renderInSettings() {
    bindCamera(getUICamera());
    foregroundColor({.99, .09, 0.09, 1});
    lightingOff();
    glDefaultObjects->currentRenderState->textScale = {4, 4};
    hudFont->renderText("Settings ", {100, scaled_height - 450, -10});

    glDefaultObjects->currentRenderState->textScale = {2, 2};

    hudFont->renderText("Graphics", {100, scaled_height - 480, -9});
    hudFont->renderText("Display", {100, scaled_height - 510, -8});
    hudFont->renderText("Controls", {100, scaled_height - 540, -7});
    hudFont->renderText("Editor", {100, scaled_height - 570, -6});
    hudFont->renderText("Back", {100, scaled_height - 600, -5});
    hudFont->renderText("Exit", {100, scaled_height - 630, -4});

    // Cursor
    int cursorY = scaled_height - (480 + (settingsMenuIndex * 30));
    hudFont->renderText("o", {50, cursorY, -3});

    glDefaultObjects->currentRenderState->textScale = {1, 1};
}

void TopDownGame::renderInEditor() {
    bindCamera(getUICamera());
    foregroundColor({.99, .09, 0.09, 1});
    lightingOff();
    glDefaultObjects->currentRenderState->textScale = {2, 2};
    hudFont->renderText("STAGE EDITOR", {2, scaled_height - 50, -10});

    glDefaultObjects->currentRenderState->textScale = {1, 1};

    static bool showStageSequenceEditor = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            // Now you can add MenuItems or other ImGui calls here
            if (ImGui::Button("New Stage Sequence")) {
                showStageSequenceEditor = true;
            }


            if (ImGui::MenuItem("Open Stage")) {
                // TODO ask if the current level should be saved?
                // TODO show file dialog for levels

            }
            if (ImGui::MenuItem("Save Level")) {
                //saveLevel();
            }
            ImGui::EndMenu();  // Close the 'File' menu
        }
        //renderMainMenu();
        ImGui::EndMainMenuBar();  // Close the main menu bar
    }
    renderWaveGraphicalEditorWindow();
    renderStageSequenceEditorWindow2(&showStageSequenceEditor);
}

void TopDownGame::renderInDisplayStageWave() {
    bindCamera(getUICamera());
    foregroundColor({.99, .09, 0.09, 1});
    lightingOff();
    glDefaultObjects->currentRenderState->textScale = {6, 6};
    std::string text = "STAGE " + std::to_string(stage+1) + " - WAVE " + std::to_string(wave+1);
    hudFont->renderText(text, {100, scaled_height - 450, -10});
    hudFont->renderText(std::to_string(stageWaveTimerCounter), {100, scaled_height - 550, -9});

    glDefaultObjects->currentRenderState->textScale = {2, 2};
}

void TopDownGame::drawPixelIntoEditorCanvas(int x, int y, glm::vec4 color, Bitmap* bitmap) {
    int offset = (x   + y * 800) * 4;
    bitmap->pixels[offset] = color.r;
    bitmap->pixels[offset+1] = color.g;
    bitmap->pixels[offset+2] = color.b;
    bitmap->pixels[offset+3] = color.a;

}

void TopDownGame::renderWaveGraphicalEditorWindow() {

    if (waveGraphicalEditorOpen) {
        std::string windowTitle = "Wave " + currentEditorWave->id;
        if (!ImGui::Begin((windowTitle + "###Wave_Graphical_Editor").c_str(), &waveGraphicalEditorOpen, ImGuiWindowFlags_MenuBar)) {
            ImGui::End();
        } else {
            // Menu bar
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Save")) {

                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            // End menu bar

            static Texture* texture = nullptr;
            if (!texture) {
                texture = createEmptyTexture(800, 600);
            }
            for (int x = 0; x < 800; x++) {
                for (int y = 0; y < 600; y++) {
                    int offset = (x   + y * 800) * 4;

                    texture->bitmap->pixels[offset] = 2;
                    texture->bitmap->pixels[offset+1] = 2;
                    texture->bitmap->pixels[offset+2] = 2;
                    texture->bitmap->pixels[offset+3] = 255;

                    // Draw a 20x20 cellsize grid
                    if (x % editorCellSize == 0 || y % editorCellSize == 0) {
                        drawPixelIntoEditorCanvas(x, y, glm::vec4(40, 40, 50, 255), texture->bitmap);
                    }

                    if (x  == 400 || y == 300) {
                        drawPixelIntoEditorCanvas(x, y, glm::vec4(80, 80, 90, 255), texture->bitmap);
                    }
                }
            }

            // Draw a rect for the arena ( or a circle...)
            {
                for (int x = 0; x < currentEditorStage->dimension.x * editorCellSize; x++) {
                    int topY = 300 + ((currentEditorStage->dimension.y / 2 ) * editorCellSize);
                    int botY = 300 - ((currentEditorStage->dimension.y / 2 ) * editorCellSize);
                    int corrX = 800 / 2 - ((currentEditorStage->dimension.x / 2) * editorCellSize) + x;
                    drawPixelIntoEditorCanvas(corrX, topY, glm::vec4(250, 240, 250, 255), texture->bitmap);
                    drawPixelIntoEditorCanvas(corrX, botY, glm::vec4(250, 240, 250, 255), texture->bitmap);
                }

                for (int y = 0; y < currentEditorStage->dimension.y * editorCellSize; y++) {
                    int leftX = 400 - ((currentEditorStage->dimension.x / 2 ) * editorCellSize);
                    int rightX = 400 + ((currentEditorStage->dimension.x / 2 ) * editorCellSize);
                    int corrY = 600 / 2 + ((currentEditorStage->dimension.y / 2) * editorCellSize) - y;
                    //corrY = (600 - editorCellSize - (currentEditorStage->dimension.y * editorCellSize)) + y;
                    drawPixelIntoEditorCanvas(leftX, corrY, glm::vec4(250, 240, 250, 255), texture->bitmap);
                    drawPixelIntoEditorCanvas(rightX, corrY, glm::vec4(250, 240, 250, 255), texture->bitmap);
                }
            }

            // Draw each enemy
            {
                for (auto ed : currentEditorWave->enemyDefinitions) {
                    for (int x = 0; x < 8; x++) {
                        for (int y = 0; y < 8; y++) {
                            drawPixelIntoEditorCanvas(
                                800/2 + (ed->initialLocation.x * editorCellSize) - 4 + x,
                                600/2 - (ed->initialLocation.y * editorCellSize) - 4 + y,
                                glm::vec4(250, 24, 10, 255),
                                texture->bitmap);
                        }


                    }
                }
            }

            updateTextTexture(texture->bitmap->width, texture->bitmap->height, texture);

            auto screenPos = ImGui::GetCursorScreenPos();
            auto mousePos = ImGui::GetMousePos();
            auto localToImage =  ImVec2( mousePos.x - screenPos.x, mousePos.y - screenPos.y);
            ImGui::Image(reinterpret_cast<ImTextureID>(texture->handle),
                      {(float) texture->bitmap->width,
                       (float) texture->bitmap->height},
                      {0, 1}, {1, 0});

            ImGui::Text("x/y %f/%f mouse: %f/%f localToImage: %f/%f", screenPos.x, screenPos.y, mousePos.x, mousePos.y, localToImage.x, localToImage.y );
            ImGui::End();
        }

    }
}

void TopDownGame::renderStageSequenceEditorWindow2(bool * p_open) {

    // Handle closing of the window, store the close state back into p_open pointer:
    if (!ImGui::Begin("StageSequence Editor", p_open, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();

    } else {

        // We shall open the window, so we set the size and position for the first time:
        ImGui::SetWindowSize("StageSequence Editor", {800, 600}, ImGuiCond_FirstUseEver);
        ImGui::SetWindowPos("StageSequence Editor", {10, 10}, ImGuiCond_FirstUseEver);

        // Small menu bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Stage Sequence")) {

                }

                if (ImGui::MenuItem("Open Stage Sequence")) {

                }

                if (ImGui::MenuItem("Save Stage Sequence")) {

                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Return if we have no stage sequence.
        if (!currentEditorstageSequence) {
            ImGui::End();
            return;
        }

        // Display the stage/wave tree:

        // ImGui::PushFont(boldFont); // Use bold font
        // ImGui::Text("Stages");
        // ImGui::PopFont(); // Revert to the previous font

        ImGui::BeginChild("StageTableContainer", ImVec2(0, 200), true);
        if (ImGui::BeginTable("stagetable", 3, ImGuiTableFlags_BordersOuter |ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {

            ImGui::TableSetupColumn("Stage");
            ImGui::TableSetupColumn("Size");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();


            for (auto stage : currentEditorstageSequence->getStages()) {
                ImGui::TableNextRow();

                if (stage == lastSelectedStageNode) {
                    ImU32 highlightColor = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 0.3f)); // Light blue
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, highlightColor);
                }

                ImGui::TableNextColumn();
                if (ImGui::Selectable(stage->id.c_str(), stage == lastSelectedStageNode)) {
                    lastSelectedStageNode = stage; // Update selected row on click
                    lastSelectedWaveNode= nullptr;
                    lastSelectedEnemyDefinition = nullptr;
                }


                char bufferx[3]; // Buffer to hold the input text
                char buffery[3]; // Buffer to hold the input text

                // Convert the integer to a string for display
                std::snprintf(bufferx, sizeof(bufferx), "%d", stage->dimension.x);
                std::snprintf(buffery, sizeof(buffery), "%d", stage->dimension.y);

                ImGui::TableNextColumn();
                if (ImGui::InputText(std::string("##width" + stage->id).c_str(), bufferx, sizeof(bufferx), ImGuiInputTextFlags_CharsDecimal)) {
                    stage->dimension.x = atoi(bufferx);
                }
                ImGui::SameLine();
                if (ImGui::InputText(std::string("##height" + stage->id).c_str(),buffery, sizeof(buffery), ImGuiInputTextFlags_CharsDecimal)) {
                    stage->dimension.y = atoi(buffery);
                }


                ImGui::TableNextColumn();
                if (ImGui::Button(std::string("Add Wave###add_wave_button" + stage->id).c_str())) {
                    stage->addWave(new Wave("foo"));
                }

            }


            ImGui::EndTable();
        }

        ImGui::EndChild();



        // Display the waves for the currently selected stage ("master/detail"):
        // ImGui::PushFont(boldFont); // Use bold font
        // ImGui::Text("Waves");
        // ImGui::PopFont(); // Revert to the previous font


        // Only show waves if we have a stage selected:
        if (!lastSelectedStageNode) {
            ImGui::End();
            return;
        }

        ImGui::BeginChild("waveTableContainer", ImVec2(0, 200), true);
        if (ImGui::BeginTable("wavetable", 2, ImGuiTableFlags_BordersOuter |ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Wave");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();

            for (auto wave : lastSelectedStageNode->getWaves()) {

                ImGui::TableNextRow();


                ImGui::Text(wave->id.c_str());

                if (wave == lastSelectedWaveNode) {
                    ImU32 highlightColor = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 0.3f)); // Light blue
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, highlightColor);
                }

                ImGui::TableNextColumn();
                if (ImGui::Selectable(wave->id.c_str(), wave == lastSelectedWaveNode)) {
                    lastSelectedWaveNode = wave; // Update selected row on click
                }
                ImGui::TableNextColumn();
                if (ImGui::Button(std::string("Add Enemy###add_enemy_button" + wave->id).c_str())) {
                    auto ed = new EnemyDefinition();
                    ed->initialLocation = {0, 0, 0};
                    wave->enemyDefinitions.push_back(ed);
                }
                ImGui::SameLine();
                if (ImGui::Button(std::string("Edit###edit_wave_button" + wave->id).c_str())) {
                    currentEditorWave = wave;
                    currentEditorStage = lastSelectedStageNode;
                    waveGraphicalEditorOpen = true;
                }


            }


            ImGui::EndTable();
        }
        ImGui::EndChild();

        if (!lastSelectedWaveNode) {
            ImGui::End();
            return;
        }

        // Enemies per wave:
        ImGui::BeginChild("EnemyTableContainer", ImVec2(0, 200), true);
        if (ImGui::BeginTable("enemytable", 5, ImGuiTableFlags_BordersOuter |ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Enemy Id");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Loc");
            ImGui::TableSetupColumn("Vel");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();

            int enemyCount = 0;
            for (auto enemy_definition : lastSelectedWaveNode->enemyDefinitions) {

                ImGui::TableNextRow();

                if (enemy_definition == currentEnemyDefinition) {
                    ImU32 highlightColor = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 0.3f)); // Light blue
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, highlightColor);
                }

                ImGui::TableNextColumn();
                if (ImGui::Selectable(std::string("###" + enemy_definition->id).c_str(), enemy_definition == lastSelectedEnemyDefinition)) {
                    lastSelectedEnemyDefinition = enemy_definition;
                }

                // Enemy id:
                {
                    char buffer[30]; // Buffer to hold the input text
                    std::snprintf(buffer, sizeof(buffer), "%s", enemy_definition->id.c_str());
                    if (ImGui::InputText(std::string("###enemy_id" + lastSelectedStageNode->id + "_" + lastSelectedWaveNode->id + std::to_string(enemyCount)).c_str(), buffer, sizeof(buffer))) {
                        enemy_definition->id = buffer;
                    }
                }


                // Type
                ImGui::TableNextColumn();
                ImGui::Text("type");

                // Location
                {
                    char bufferx[5]; // Buffer to hold the input text
                    char buffery[5]; // Buffer to hold the input text

                    // Convert the integer to a string for display
                    std::snprintf(bufferx, sizeof(bufferx), "%d", enemy_definition->initialLocation.x);
                    std::snprintf(buffery, sizeof(buffery), "%d", enemy_definition->initialLocation.y);

                    ImGui::TableNextColumn();
                    if (ImGui::InputText(std::string("###width" + enemy_definition->id).c_str(), bufferx, sizeof(bufferx), ImGuiInputTextFlags_CharsDecimal)) {
                        enemy_definition->initialLocation.x = atoi(bufferx);
                    }
                    ImGui::SameLine();
                    if (ImGui::InputText(std::string("###height" + enemy_definition->id).c_str(),buffery, sizeof(buffery), ImGuiInputTextFlags_CharsDecimal)) {
                        enemy_definition->initialLocation.y = atoi(buffery);
                    }
                }

                // Velocity
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("vel.");
                }




                ImGui::TableNextColumn();
                if (ImGui::Button(std::string("Edit###edit_enemy_button" + enemy_definition->id).c_str())) {


                }


                enemyCount++;
            }


            ImGui::EndTable();
        }
        ImGui::EndChild();



        ImGui::End();
    }

}

void TopDownGame::renderStageSequenceEditorWindow(bool * p_open) {

    if (!ImGui::Begin("StageSequence Editor", p_open, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();

    } else {
        ImGui::SetWindowSize("StageSequence Editor", {800, 600}, ImGuiCond_FirstUseEver);
        ImGui::SetWindowPos("StageSequence Editor", {10, 10}, ImGuiCond_FirstUseEver);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save")) {

                }
                if (ImGui::MenuItem("Save as...")) {

                }


                if (ImGui::MenuItem("+New Stage")) {

                }

                if (ImGui::MenuItem("Playtest")) {
                    // TODO show file dialog for levels

                }
                ImGui::EndMenu();  // Close the 'File' menu
            }

            if (ImGui::BeginMenu("Test")) {
                if (ImGui::Button("Play stage")) {

                }
                ImGui::EndMenu();

            }

            ImGui::EndMenuBar();
        }



        // Render a tree of stages and waves
        if (ImGui::TreeNode("stage sequence")) {
            int stageCount=0;
            for (auto stage : currentEditorstageSequence->getStages()) {

                int waveCount = 0;
                bool open = ImGui::TreeNode(std::string("stage " + std::to_string(stageCount)).c_str());

                if (ImGui::BeginPopupContextItem())
                {
                    // Context menu for each stage
                    if (ImGui::MenuItem("Add wave..")) {
                        stage->addWave(new Wave(std::string("s") + std::to_string(stageCount) + "w" + std::to_string(waveCount)));

                    }

                    ImGui::EndPopup();
                }





                if (open) {

                    if (ImGui::IsItemClicked()) {
                        lastSelectedStageNode = stage;
                    }

                    for (auto wave : stage->getWaves()) {
                        bool waveOpen = ImGui::TreeNode(std::string("wave" + std::to_string(waveCount)).c_str());

                        if (ImGui::BeginPopupContextItem())
                        {
                            // Context menu for each stage
                            if (ImGui::Button("Add enemy...")) {
                                wave->enemyDefinitions.push_back(new EnemyDefinition());
                            }

                            if (ImGui::MenuItem("Modify in graphical editor")) {
                                waveGraphicalEditorOpen = true;
                                currentEditorWave = wave;
                                currentEditorStage = stage;
                            }
                            ImGui::EndPopup();
                        }

                        if (waveOpen) {
                            if (ImGui::IsItemClicked()) {
                                lastSelectedWaveNode = wave;
                            }
                            int enemyCount = 0;
                            for (auto ed : wave->enemyDefinitions) {
                                bool enemyOpen = ImGui::TreeNode(std::string("Enemy_" + std::to_string(enemyCount)).c_str());
                                if (enemyOpen) {

                                    if (ImGui::IsItemClicked()) {
                                        lastSelectedEnemyDefinition = ed;
                                    }

                                    ImGui::TreePop();
                                }
                                enemyCount++;
                            }

                            ImGui::TreePop();
                        }



                        waveCount++;
                    }


                    ImGui::TreePop();

                }

                stageCount++;
            }
            ImGui::TreePop();
        }

        ImGui::Text("Stage Properties:");
        if (lastSelectedStageNode) {



            if (ImGui::BeginTable("StagePropsTable", 2)) {

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Id");
                ImGui::TableNextColumn();
                ImGui::Text(lastSelectedStageNode->id.c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Width");
                ImGui::TableNextColumn();
                ImGui::InputInt("##width", &lastSelectedStageNode->dimension.x);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Height");
                ImGui::TableNextColumn();
                ImGui::InputInt("##height", &lastSelectedStageNode->dimension.y);

                ImGui::EndTable();
            }

        }

        ImGui::Text("Wave Properties:");
        if (lastSelectedWaveNode) {
            ImGui::Text("Current selected Wave: %s", lastSelectedWaveNode->id.c_str());

            if (ImGui::BeginTable("wave_props_table", 2)) {

                ImGui::EndTable();
            }
        }

        ImGui::Text("Enemy properties:");
        if (lastSelectedEnemyDefinition) {
            ImGui::Text("Current selected enemy: %p", lastSelectedEnemyDefinition);

            if (ImGui::BeginTable("enemy_props_table", 2)) {

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Type");
                ImGui::TableNextColumn();
                ImGui::Text("%d", &lastSelectedEnemyDefinition->type);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Starting position X");
                ImGui::TableNextColumn();
                ImGui::InputInt("##start_pos_x", &lastSelectedEnemyDefinition->initialLocation.x);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Starting position Y");
                ImGui::TableNextColumn();
                ImGui::InputInt("##start_pos_y", &lastSelectedEnemyDefinition->initialLocation.y);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Starting velocity X");
                ImGui::TableNextColumn();
                ImGui::InputInt("##start_vel_x", &lastSelectedEnemyDefinition->initialVelocity.x);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Starting velocity Y");
                ImGui::TableNextColumn();
                ImGui::InputInt("##start_vel_y", &lastSelectedEnemyDefinition->initialVelocity.y);
                ImGui::EndTable();

            }
        }


        ImGui::End();
    }


}

Enemy::Enemy(BulletPattern *bulletPattern) : bulletPattern(bulletPattern) {
}

DefaultGame* getGame() {
    return new TopDownGame();
}


