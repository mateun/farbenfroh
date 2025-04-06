//
// Created by mgrus on 30.03.2025.
//

#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H
#include <complex.h>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/Widget.h>
#include <engine/graphics/Texture.h>


class ButtonWidget : public Widget {

public:
    ButtonWidget();
    void draw(float depth = -0.5) override;
    MessageHandleResult onMessage(const UIMessage &message) override;
    glm::vec2 getPreferredSize() override;
    void setTexture(std::shared_ptr<Texture> tex);

    void setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) override;
    void removeHoverFocus() override;

private:

    void setLastProcessedMessage(uint64_t lastProcessedMessage);
    bool hover_ = false;
    std::shared_ptr<Texture> texture_;
    bool hover_focus_ = false;
    uint64_t last_processed_message_num_ = 0;
};



#endif //BUTTONWIDGET_H
