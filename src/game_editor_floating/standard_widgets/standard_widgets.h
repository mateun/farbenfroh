//
// Created by mgrus on 15.04.2025.
//

#ifndef STANDARD_WIDGETS_H
#define STANDARD_WIDGETS_H
#include "../d2d_ui.h"

struct d2d_CloseButtonWidget : public d2d_Widget {
  void draw() override;
  void onClick() override;

};

#endif //STANDARD_WIDGETS_H
