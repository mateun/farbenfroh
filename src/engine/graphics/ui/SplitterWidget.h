//
// Created by mgrus on 29.03.2025.
//

#ifndef SPLITTERWIDGET_H
#define SPLITTERWIDGET_H

#include <memory>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/Widget.h>


enum class SplitterType {
  Horizontal,
  Vertical
};

/**
* This Widgt takes two widgets as children and offers the possibility
* for the user to change the size distribution between the widgets.
* Splitters can be horizontal or vertical.
* This class ignores items added via "addChild" - only the explicit first and second item are considered.
*
*/
class SplitterWidget : public Widget {
  public:
    SplitterWidget(SplitterType type, std::shared_ptr<Widget> first, std::shared_ptr<Widget> second);
    void draw() override;

    MessageHandleResult onMessage(const UIMessage &message) override;

    SplitterType type_;
    bool mouse_over_splitter_ = false;
    bool mouse_down_ = false;

  private:
  std::shared_ptr<Widget> first_;
  std::shared_ptr<Widget> second_;
  glm::vec2 splitterPosition_ = glm::vec2(0.0f);
};



#endif //SPLITTERWIDGET_H
