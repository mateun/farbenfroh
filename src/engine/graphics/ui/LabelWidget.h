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
    void draw() override;
    glm::vec2 getPreferredSize() override;
    glm::vec2 getMinSize() override;
    glm::vec2 getMaxSize() override;

    // Label specific properties
    void setText(const std::string & text);


private:
    std::string text_;
    std::unique_ptr<Mesh> quadMesh;
    std::shared_ptr<TrueTypeTextRenderer> textRenderer_;
    std::shared_ptr<TrueTypeFont> font_;

};



#endif //LABELWIDGET_H
