//
// Created by mgrus on 15.02.2025.
//

#ifndef UPDATESWITCHER_H
#define UPDATESWITCHER_H

#include <vector>

class Updatable {
  public:
  virtual void update() = 0;

};




/**
* This class can round robin switch between different classes update method.
* Example is to direct input between a (debug) camera and the actual character.
*/
class UpdateSwitcher {

public:
    UpdateSwitcher(const std::vector<Updatable*>& _sceneNodes, int triggerKey);

    void addTarget(Updatable* target);
    void removeTarget(Updatable* target);
    void update();

private:

    void poll();

    std::vector<Updatable *> _nodes;
    int _triggerKey = -1;
    Updatable* _currentUpdatable = nullptr;
};
#endif //UPDATESWITCHER_H
