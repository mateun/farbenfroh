//
// Created by mgrus on 07.04.2025.
//

#ifndef ROUNDEDRECT_H
#define ROUNDEDRECT_H

#include <engine/graphics/Widget.h>

class MessageHandleResult;

class RoundedRect : public Widget {
public:
    explicit RoundedRect(float cornerRadius = 18);
    ~RoundedRect() override = default;

    void draw(float depth) override;



private:
    float corner_radius_;
};



#endif //ROUNDEDRECT_H
