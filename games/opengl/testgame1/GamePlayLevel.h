//
// Created by mgrus on 25.02.2025.
//

#ifndef GAMEPLAY_H
#define GAMEPLAY_H

class DefaultApp;

class GamePlayLevel : public GameLevel {

public:
  GamePlayLevel(DefaultApp* game);
  void render() override;
  void update() override;
  void init() override;

};



#endif //GAMEPLAY_H
