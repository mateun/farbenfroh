//
// Created by mgrus on 30.03.2025.
//

#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H
#include <complex.h>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/Widget.h>
#include <engine/graphics/Texture.h>


class LabelWidget;
class RoundedRect;

/**
* The button either renders as a rounded rect with text on it.
* Or with a texture, where the texture size gives the size of the button.
*/
class ButtonWidget : public Widget {

public:
    // For Texture based buttons
    ButtonWidget(std::shared_ptr<Texture> texture);

    // For Text based buttons
    ButtonWidget(const std::string &text, const std::shared_ptr<TrueTypeFont> &font);

    void draw(float depth = -0.5) override;
    MessageHandleResult onMessage(const UIMessage &message) override;
    glm::vec2 getPreferredSize() override;
    void setTexture(std::shared_ptr<Texture> tex);

    void setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) override;
    void removeHoverFocus() override;

    void enable();
    void disable();


private:

    void setLastProcessedMessage(uint64_t lastProcessedMessage);
    void drawTexturedButton(float depth);
    void drawTextButton(float depth);
    void drawHoverUnderlay(float depth);

    bool hover_ = false;
    std::shared_ptr<Texture> texture_;
    bool hover_focus_ = false;
    uint64_t last_processed_message_num_ = 0;
    std::string text_;
    std::shared_ptr<RoundedRect> round_rect_;
    std::shared_ptr<LabelWidget> label_widget_;
    std::shared_ptr<TrueTypeFont> font_;
    bool enabled_ = true;
};



#endif //BUTTONWIDGET_H
