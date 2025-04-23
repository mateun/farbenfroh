//
// Created by mgrus on 31.03.2025.
//


#include <engine/graphics/Widget.h>
#include <engine/graphics/ui/MenuBar.h>

float Layout::getTopmostY(Widget* target) const {

    // We must check if the target is actually a MenuBar.
    // If not, we do the normal check and see if it has a menuBar
    if (dynamic_cast<MenuBar*>(target) == nullptr) {
        return target->hasMenuBar() ? target->size().y - 32 : target->origin().y + target->size().y;
    }


    return target->origin().y + target->size().y;


}
