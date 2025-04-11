//
// Created by mgrus on 27.03.2025.
//

#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include <complex.h>
#include <string>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/TrueTypeTextRenderer.h>
#include <engine/graphics/Widget.h>

class LabelWidget : public Widget {

public:
    LabelWidget(const std::string & text, const std::shared_ptr<TrueTypeFont> & font);
    void draw(float depth = -0.5) override;
    glm::vec2 getPreferredSize() override;
    glm::vec2 getMinSize() override;
    glm::vec2 getMaxSize() override;

    MessageHandleResult onMessage(const UIMessage &message) override;

    // Label specific properties
    void setText(const std::string & text);

    void setTextColor(glm::vec4 text_color);

    TextDimensions calculateSizeForText(const std::string& str);

    void setHoverFocus(std::shared_ptr<Widget> prevFocusHolder) override;

private:
    std::string text_;
    std::unique_ptr<Mesh> quadMesh;
    std::shared_ptr<TrueTypeTextRenderer> textRenderer_;
    std::shared_ptr<TrueTypeFont> font_;
    glm::vec4 text_color_ = glm::vec4(0.8, 0.82, 0.8, 1.0f);
};




#endif //LABELWIDGET_H
