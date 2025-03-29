//
// Created by mgrus on 29.03.2025.
//

#include "SplitterWidget.h"

#include <engine/graphics/Widget.h>


SplitterWidget::SplitterWidget(SplitterType type, Widget *first, Widget *second): type_(type), first_(first),
    second_(second) {
}

void SplitterWidget::draw() {
    Widget::draw();
}
