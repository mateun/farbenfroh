//
// Created by mgrus on 27.03.2025.
//

#include <engine/graphics/Widget.h>

void VBoxLayout::apply(Widget *target) {

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
    // Otherwise we need to crop or scroll.f
    float runningVerticalSum = 0;
    if (maxPrefX <= target->size().x &&
        maxPrefY <= target->size().y) {
        // All good, lets give the preferred size to the widdgets:
        // TODO: we should probably introduce additional logic here for expanding widgets:
        for (auto c : target->children()) {
            auto preferredSize = c->getPreferredSize();
            bool expandVertical = c->getLayoutHint().expandVertically;
            if (expandVertical) {
                // TODO also applying horizontal expansion here.. I know..
                c->setSize({target->size().x, target->size().y - runningVerticalSum});

            } else {
                c->setSize(c->getPreferredSize());
                runningVerticalSum += preferredSize.y;
            }

        }

        // Assign the positions from top to bottom

        int counter = 1;
        float verticalCursor = topY - margin_vertical_;
        for (auto c : target->children()) {
            // Widgets are drawn bottom-up so we need to deduct the size of the widget for the next origin.y position:
            verticalCursor -= c->size().y;
            c->setOrigin({target->origin().x + margin_horizontal_, verticalCursor});
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
        if (minMinX >= target->size().x && maxMinX <= target->size().x &&
        minMinY >= target->size().y && maxMinY <= target->size().y) {
            // At least everyone is fitting with their minimum sizes.
            for (auto c : target->children()) {
                c->setSize(c->getMinSize());
            }

            float topY = target->size().y;
            int counter = 1;
            for (auto c : target->children()) {
                c->setOrigin({margin_horizontal_, topY - margin_vertical_ - (counter * c->getMinSize().y)});
                counter++;
            }
        }
    }

}

void VBoxLayout::setMarginHorizontal(int margin) {
    margin_horizontal_ = margin;
}

void VBoxLayout::setMarginVertical(int margin) {
    margin_vertical_ = margin;
}
