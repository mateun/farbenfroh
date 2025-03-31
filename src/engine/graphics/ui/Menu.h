//
// Created by mgrus on 31.03.2025.
//

#ifndef MENU_H
#define MENU_H

#include <engine/graphics/Widget.h>

class LabelWidget;


/**
* A menu has a title.
*/
class Menu : public Widget {
public:
  Menu(const std::string &text);

  void draw() override;
  MessageHandleResult onMessage(const UIMessage &message) override;
  glm::vec2 getPreferredSize() override;

private:
  std::string text_;
  std::shared_ptr<LabelWidget> label_;


};



#endif //MENU_H
