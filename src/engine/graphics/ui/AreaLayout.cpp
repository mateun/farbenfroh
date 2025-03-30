//
// Created by mgrus on 30.03.2025.
//

#include <engine/graphics/PlanePivot.h>
#include <engine/graphics/Widget.h>

AreaLayout::AreaLayout(std::shared_ptr<Widget> top, std::shared_ptr<Widget> bottom, std::shared_ptr<Widget> left,
    std::shared_ptr<Widget> right, std::shared_ptr<Widget> center): top_(std::move(top)), bottom_(std::move(bottom)), left_(std::move(left)),
                                    right_(std::move(right)), center_(std::move(center)) {
}

void AreaLayout::apply(Widget *target) {
    auto preferredVerticalTop = 0;
    auto preferredVerticalBottom = 0;
    auto preferredHorizontalLeft = 0;
    auto preferredHorizontalRight = 0;
    auto preferredVerticalCenter = 0;

    // First lets see how much vertical space the top item wants:
    if (top_) {
        preferredVerticalTop = top_->getPreferredSize().y;

        // Is this less than our own height,
        // then we can give the preferred height to the widget.
        // TODO: refer also to bottom. Both might be not big enough?
        // Sum of top.y and bottom.y, then check?!
        if (preferredVerticalTop < target->size().y) {
            top_->setSize({target->size().x, top_->getPreferredSize().y});
            top_->setOrigin({target->origin().x, target->getPreferredSize().y - preferredVerticalTop});
        }
    }

    if (center_) {
        if (top_) {
            center_->setSize({target->size().x, target->size().y - top_->getPreferredSize().y});
        }
        center_->setOrigin(target->origin());

    }

    // TODO handle bottom, left, right...


}
