//
// Created by mgrus on 02.04.2025.
//

#ifndef SPACER_H
#define SPACER_H

#include <engine/graphics/Widget.h>



class Spacer : public Widget {

public:
    Spacer(glm::vec2 size);
    glm::vec2 getPreferredSize() override;
    MessageHandleResult onMessage(const UIMessage &message) override;

private:
    glm::vec2 preferred_size_;

};



#endif //SPACER_H
