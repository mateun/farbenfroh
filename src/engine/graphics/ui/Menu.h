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
class Menu : public Widget{
public:
  Menu(const std::string &text);


  void addSubMenu(std::shared_ptr<Menu> subMenu);
  void addMenuItem(std::shared_ptr<MenuItem> menuItem);



  void draw(float depth = -0.5) override;
  MessageHandleResult onMessage(const UIMessage &message) override;
  glm::vec2 getPreferredSize() override;
  void setParent(std::weak_ptr<Menu> parent);
  std::weak_ptr<Menu> parentMenu();

  void setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) override;
  void removeHoverFocus() override;

  void closeSubMenuPanel();

  bool isWidgetAChild(const std::shared_ptr<Menu> & menu);

  //void widgetGotHoverFocus(std::shared_ptr<Widget> widget) override;

  // Checks if "this" is any ancestor (parent, grand-parent..) of the given reference menu.
  bool isAncestorOf(const std::shared_ptr<Menu> & referenceWidget);

private:
  std::weak_ptr<Menu> parent_menu_;
  std::string text_;
  std::shared_ptr<LabelWidget> label_;
  bool hover_ = false;
  std::vector<std::shared_ptr<Menu>> sub_menus_;
  std::vector<std::shared_ptr<MenuItem>> items_;
  std::shared_ptr<Widget> sub_menu_panel_;

  bool sub_menu_open_ = false;
  bool hover_sub_panel_ = false;

  bool app_hover_focus_ = false;

  void lazyCreateSubMenuPanel();
};




#endif //MENU_H
