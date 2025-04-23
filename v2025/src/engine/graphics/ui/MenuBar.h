//
// Created by mgrus on 31.03.2025.
//

#ifndef MENUBAR_H
#define MENUBAR_H
#include <memory>
#include <vector>
#include <engine/graphics/Widget.h>


struct UIMessage;
class Menu;
struct MessageHandleResult;


/**
* A MenuBar can be attached to a Widget and will occupy to the topmost row.
* MenuBars can have menus, each menu has MenuItems.
*/
class MenuBar : public Widget {
  public:
    MenuBar();
    ~MenuBar() = default;
    void addMenu(std::shared_ptr<Menu> menu);
    void draw(float depth) override;
    MessageHandleResult onMessage(const UIMessage &message) override;

private:
    std::vector<std::shared_ptr<Menu>> menus_;
};



#endif //MENUBAR_H
