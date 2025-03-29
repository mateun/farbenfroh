//
// Created by mgrus on 29.03.2025.
//

#ifndef SPLITTERWIDGET_H
#define SPLITTERWIDGET_H

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
    SplitterWidget(SplitterType type, Widget* first, Widget* second);
    void draw() override;

    SplitterType type_;

private:
  Widget* first_;
  Widget* second_;
};



#endif //SPLITTERWIDGET_H
