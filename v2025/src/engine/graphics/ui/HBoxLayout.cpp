//
// Created by mgrus on 27.03.2025.
//

#define NOMINMAX
#include <engine/graphics/Widget.h>

void HBoxLayout::apply(Widget *target) {

    float minPrefX = std::numeric_limits<float>::max();
    float minPrefY = std::numeric_limits<float>::max();
    float maxPrefX = std::numeric_limits<float>::min();
    float maxPrefY = std::numeric_limits<float>::min();

    float topY = getTopmostY(target);

    // 1. Collect the preferred sizes
    for (auto c : target->children()) {
        auto ps = c->getPreferredSize();

        if (ps.x < minPrefX) minPrefX = ps.x;
        if (ps.x > maxPrefX) maxPrefX = ps.x;
        if (ps.y < minPrefY) minPrefY = ps.y;
        if (ps.y > maxPrefY) maxPrefY = ps.y;
    }

    // 2. Compare to my own size, does this fit?
    // If yes, we are fine, every child can get the size it wants.
    // If no, we need to look at the min. sizes and see if these fit.
    // Otherwise we need to crop or scroll.
    if (maxPrefX < target->size().x &&
        maxPrefY < target->size().y) {
        // All good, lets give the preferred size to the widdgets:
        for (auto c : target->children()) {
            c->setSize(c->getPreferredSize());
        }

        // Assign the positions from left to right
        // We assume a left alignment for now, so that every
        // new widget is attached to the preceding one, with the
        // horizontal margin in between.
        int counter = 0;
        int cursorX = target->origin().x;
        for (auto c : target->children()) {
            c->setOrigin({cursorX, target->origin().y + margin_vertical_});
            cursorX += c->getPreferredSize().x + margin_horizontal_;
            counter++;
        }
    }
    else {
        // Collect all min sizes:
        float minMinX = std::numeric_limits<float>::max();
        float minMinY = std::numeric_limits<float>::max();
        float maxMinX = std::numeric_limits<float>::min();
        float maxMinY = std::numeric_limits<float>::min();
        for (auto c : target->children()) {
            auto ps = c->getMinSize();
            if (ps.x < minMinX) minMinX = ps.x;
            if (ps.x > maxMinX) maxMinX = ps.x;
            if (ps.y < minMinY) minMinY = ps.y;
            if (ps.y > maxMinY) maxMinY = ps.y;
        }
        if (minMinX > target->size().x && maxMinX < target->size().x &&
        minMinY > target->size().y && maxMinY < target->size().y) {
            // At least everyone is fitting with their minimum sizes.
            for (auto c : target->children()) {
                c->setSize(c->getMinSize());
            }

            int counter = 0;
            for (auto c : target->children()) {
                c->setOrigin({target->origin().x + margin_horizontal_ + (counter * c->getMinSize().x) , margin_vertical_});
                counter++;
            }
        }
    }

}


void HBoxLayout::setMarginHorizontal(int margin) {
    margin_horizontal_ = margin;
}

void HBoxLayout::setMarginVertical(int margin) {
    margin_vertical_ = margin;
}
