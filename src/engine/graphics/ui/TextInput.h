//
// Created by mgrus on 07.04.2025.
//

#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include <engine/graphics/Widget.h>

class TrueTypeFont;
class LabelWidget;

// Single line text input.
class TextInput : public Widget {

public:
    TextInput(const std::string& initialText, const std::shared_ptr<TrueTypeFont>& font);
    ~TextInput();

    void draw(float depth) override;
    MessageHandleResult onMessage(const UIMessage &message) override;

    glm::vec2 getPreferredSize() override;

    void setTextColor(glm::vec4 color);



private:
    std::string text_;
    int cursor_pos_ = 0;
    std::shared_ptr<LabelWidget> widget_;
    std::shared_ptr<TrueTypeFont> font_;
    std::shared_ptr<LabelWidget> label_widget_;
    glm::vec4 text_color_ = {0, 0,0, 1};
};



#endif //TEXTINPUT_H
