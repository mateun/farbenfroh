//
// Created by mgrus on 25.02.2025.
//

#ifndef GAMEPLAY_H
#define GAMEPLAY_H

class DefaultGame;

class GamePlayLevel : public GameLevel {

public:
  GamePlayLevel(DefaultGame* game);
  void render() override;
  void update() override;
  void init() override;

  Scene * scene = nullptr;
  SceneNode * playerNode = nullptr;
  AnimationPlayer * idlePlayer = nullptr;
  AnimationPlayer * walkPlayer = nullptr;
  AnimationController * animationController = nullptr;
};



#endif //GAMEPLAY_H
