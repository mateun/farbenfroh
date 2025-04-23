//
// Created by mgrus on 15.04.2025.
//

#ifndef STANDARD_WIDGETS_H
#define STANDARD_WIDGETS_H
#include <d2d1.h>
#include <d2d1_3.h>
#include <functional>
#include <map>

#include "../d2d_ui.h"
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

enum class TextAlignHorizontal { Left, Center, Right };
enum class TextAlignVertical { Top, Middle, Bottom };

struct d2d_FontCache {
  IDWriteTextFormat* get_or_create(const std::wstring& fontName, float size, TextAlignHorizontal horizontal, TextAlignVertical vertical);

private:
  std::map<std::tuple<std::wstring, float, TextAlignHorizontal, TextAlignVertical>, IDWriteTextFormat*> formats_;
};

struct d2d_Label : d2d_Widget {
  d2d_Label(const std::wstring& text);
  std::wstring text_;
  glm::vec4 text_color_;
  std::wstring font_name_ = L"../assets/calibri.ttf";
  float font_size_ = 14.0f;
  TextAlignHorizontal align_horizontal_ = TextAlignHorizontal::Left;
  TextAlignVertical align_vertical_ = TextAlignVertical::Middle;
  void draw() override;

private:
  IDWriteTextFormat * cached_text_format_ = nullptr;



};

struct d2d_Button : d2d_Widget {
  d2d_Button(D2D1_RECT_F bounds);
  d2d_Label* label_ = nullptr;
  glm::vec4 bg_color_ = glm::vec4(255, 0, 255, 255);
  glm::vec4 bg_hover_color_ = glm::vec4(225, 0, 225, 255);
  std::function<void()> click_callback_;
  void draw() override;
  void onClick() override;

};

struct d2d_CloseButtonWidget : d2d_Widget {
  void draw() override;
  void onClick() override;

};

enum class GripOrientation {
  Horizontal, // top or bottom
  Vertical    // left or right
};

struct d2d_GripWidget : d2d_Widget {
  d2d_GripWidget(HWND window_to_drag);

  GripOrientation orientation_ = GripOrientation::Horizontal;
  HWND hwnd_to_drag_ = nullptr;

  glm::vec4 dotColor = { 0.5f, 0.5f, 0.5f, 1.0f };
  float dotRadius = 2.0f;
  float spacing = 6.0f;

  bool dragging = false;
  glm::vec2 dragStartMouse;
  glm::vec2 dragStartWindow;
  glm::vec2 drag_start_mouse_;
  glm::vec2 drag_start_window_;

  void draw() override;
  HCURSOR get_cursor() const override;
  void on_mouse_down(int mouse_x, int mouse_y) override;
  void on_mouse_move(int mouse_x, int mouse_y) override;
  void on_mouse_up(int mouse_x, int mouse_y) override;
  bool wants_capture_on_mouse_down() const override;


};

struct d2d_ToolbarIconButton : d2d_Widget {
  d2d_ToolbarIconButton(const std::wstring& svg_icon_file);
  std::wstring svg_file_name_;

  void draw() override;

private:
  ID2D1SvgDocument* svg_icon = nullptr;
};

#endif //STANDARD_WIDGETS_H
