//
// Created by mgrus on 27.03.2025.
//

#include <engine/graphics/Widget.h>

glm::vec2 VBoxLayout::calculateChildOrigin(const Container *parent, const Widget *child) {
    int counter = 0;
    for (auto c : parent->children()) {
        c->setOrigin(0, counter * 20);
        if (c.get() == child) {
            return c->origin();
        }
        counter++;

        // if (Container* container = dynamic_cast<Container*>(c.get())) {
        //     // widget is a Container (or derived from Container)
        // } else {
        //     // widget is not a Container
        // }

    }
}

glm::vec2 VBoxLayout::calculateChildSize(const Container *parent, const Widget *child) {
    return Layout::calculateChildSize(parent, child);
}
