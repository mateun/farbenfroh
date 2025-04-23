//
// Created by mgrus on 07.04.2025.
//

#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include <complex.h>
#include <engine/graphics/Widget.h>

#include "RoundedRect.h"

class TrueTypeFont;
class LabelWidget;

// Single line text input.
class TextInput : public Widget {

public:
    TextInput(const std::string& initialText, const std::shared_ptr<TrueTypeFont>& font);

    float charCursorToPixelPos();

    ~TextInput() override = default;

    void addTextChangeListener(std::function<void(std::shared_ptr<TextInput>)> listener);

    void draw(float depth) override;
    void drawCursor(float depth);

    void invokeTextListenerCallbacks();

    MessageHandleResult onMessage(const UIMessage &message) override;

    glm::vec2 getPreferredSize() override;

    void setTextColor(glm::vec4 color);

    void setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) override;
    void removeHoverFocus() override;

    std::string getText();

private:
    std::string text_;
    std::shared_ptr<LabelWidget> widget_;
    std::shared_ptr<TrueTypeFont> font_;
    std::shared_ptr<LabelWidget> label_widget_;
    std::shared_ptr<RoundedRect> input_field_;
    glm::vec4 text_color_ = {0, 0,0, 1};
    bool hover_focus_ = false;
    bool render_cursor_ = false;
    int char_cursor_pos_ = 0;
    uint64_t prev_message_num_ = 0;
    std::vector<std::function<void(std::shared_ptr<TextInput>)>> text_change_listeners_;
    bool blink_timer_ready_ = true;
    float blink_timer_ = 0;
    std::string initial_text_;
    bool fit_to_text_ = false;
};



#endif //TEXTINPUT_H
