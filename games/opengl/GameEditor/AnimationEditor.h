//
// Created by mgrus on 23.03.2025.
//

#ifndef ANIMATIONEDITOR_H
#define ANIMATIONEDITOR_H

#include <engine/graphics/Application.h>


class AnimationEditor : public Application {
public:
    AnimationEditor(int width, int height, bool fullscreen);

    virtual void onCreated() override;
};



#endif //ANIMATIONEDITOR_H
