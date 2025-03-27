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
    LabelWidget(const std::string & text);
    void draw(Camera* camera) override;

private:
    std::string text_;
    std::unique_ptr<Mesh> quadMesh;
    std::shared_ptr<TrueTypeTextRenderer> textRenderer_;
};



#endif //LABELWIDGET_H
