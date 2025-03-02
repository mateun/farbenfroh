//
// Created by mgrus on 25.02.2025.
//

#ifndef MAINMENUSCENE_H
#define MAINMENUSCENE_H
#include "../../../src/engine/game/default_game.h"

class DefaultGame;

class MainMenuLevel : public GameLevel  {

public:
  MainMenuLevel(DefaultGame* game);
  void render() override;
  void update() override;
  void init() override;


private:
  Shader * mechShader = nullptr;
  CameraMover * cameraMover = nullptr;
  Scene * scene = nullptr;
  Light * sun = nullptr;
  Cinematic* cameraCinematic = nullptr;
  CharacterController * characterController = nullptr;
  UpdateSwitcher* updateSwitcher = nullptr;
  AnimationPlayer * idlePlayer = nullptr;
  AnimationPlayer * walkPlayer = nullptr;
  SceneNode * playerNode = nullptr;
  Shader * skinnedShader = nullptr;
  AnimationController * animationController = nullptr;
  Cinematic * mechFlyCinematic = nullptr;
  gru::Bvh * playerBvh = nullptr;
  gru::Bvh * mechBvh = nullptr;
};



#endif //MAINMENUSCENE_H
