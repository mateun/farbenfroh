//
// Created by mgrus on 31.03.2025.
//

#ifndef MENUBAR_H
#define MENUBAR_H
#include <engine/graphics/Widget.h>

class Menu;
/**
* A MenuBar can be attachted to a Widget and will occupy to the topmost row.
* MenuBars can have menus, each menu has MenuItems.
*/
class MenuBar : public Widget {
  public:
    MenuBar();
    void addMenu(std::shared_ptr<Menu> menu);
    void draw(float depth = -0.5) override;
    MessageHandleResult onMessage(const UIMessage &message) override;

private:
    std::vector<std::shared_ptr<Menu>> menus_;
};



#endif //MENUBAR_H
