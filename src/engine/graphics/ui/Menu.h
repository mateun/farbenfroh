//
// Created by mgrus on 31.03.2025.
//

#ifndef MENU_H
#define MENU_H

#include <engine/graphics/Widget.h>
#include <vector>

class LabelWidget;
class MenuItem;

/**
* A menu has a title.
*/
class Menu : public Widget {
public:
  Menu(const std::string &text);


  void addSubMenu(std::shared_ptr<Menu> subMenu);
  void addMenuItem(std::shared_ptr<MenuItem> menuItem);

  void draw() override;
  MessageHandleResult onMessage(const UIMessage &message) override;
  glm::vec2 getPreferredSize() override;

private:
  std::string text_;
  std::shared_ptr<LabelWidget> label_;
  bool hover_ = false;
  std::vector<std::shared_ptr<Menu>> sub_menus_;
  std::vector<std::shared_ptr<MenuItem>> items_;
  std::shared_ptr<Widget> sub_menu_panel_;

  void lazyCreateSubMenuPanel();
};



#endif //MENU_H
