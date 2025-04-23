//
// Created by mgrus on 05.04.2025.
//

#ifndef ACTION_H
#define ACTION_H
#include <memory>

#include <engine/graphics/Widget.h>

/**
* Actions can be attached to buttons, menus etc.
* Things the user can interact with and trigger the respective action.
*/
class Action {

  public:
    explicit Action(const std::string& id);
    virtual void execute(std::shared_ptr<Widget> source) = 0;

private:
    const std::string id_;
};



#endif //ACTION_H
