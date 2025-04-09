//
// Created by mgrus on 30.03.2025.
//

#include <engine/graphics/PlanePivot.h>
#include <engine/graphics/Widget.h>

AreaLayout::AreaLayout() {
    createDummyParts();
}

void AreaLayout::createDummyParts() {
    // Create default placeholder widgets for non-existing widgets:
    if (!top_) {
        top_ = std::make_shared<Widget>();
        top_->setId("empty_top_");
    }

    if (!bottom_) {
        bottom_ = std::make_shared<Widget>();
        bottom_->setId("empty_bottom_");
    }

    if (!left_) {
        left_ = std::make_shared<Widget>();
        left_->setId("empty_left_");
    }

    if (!right_) {
        right_ = std::make_shared<Widget>();
        right_->setId("empty_right_");
    }
    if (!center_) {
        center_ = std::make_shared<Widget>();
        center_->setId("empty_center_");
    }
}

AreaLayout::AreaLayout(std::shared_ptr<Widget> top, std::shared_ptr<Widget> bottom, std::shared_ptr<Widget> left,
                       std::shared_ptr<Widget> right, std::shared_ptr<Widget> center): top_(std::move(top)), bottom_(std::move(bottom)), left_(std::move(left)),
                                                                                       right_(std::move(right)), center_(std::move(center)) {

    createDummyParts();

}

void AreaLayout::apply(Widget *target) {

    auto top_and_bottom_height = top_->getPreferredSize().y+ bottom_->getPreferredSize().y;
    if (top_and_bottom_height <= target->size().y) {
        top_->setSize({target->size().x, top_->getPreferredSize().y});
        bottom_->setSize({target->size().x, bottom_->getPreferredSize().y});
    } else {
        // Assign a height to top and bottom, which gives both enough room -
        // at the cost of the center height.
        top_->setSize({target->size().x, target->size().y / 2});
        bottom_->setSize({target->size().x / 2, target->size().y / 2});
    }

    top_->setOrigin({target->origin().x, (target->origin().y + target->size().y) - top_->size().y});
    bottom_->setOrigin({target->origin().x, target->origin().y});

    auto left_and_right_width = left_->getPreferredSize().x - right_->getPreferredSize().x;
    if (left_and_right_width <= target->size().x) {
        left_->setSize({left_->getPreferredSize().x, target->size().y - (top_->size().y + bottom_->size().y)});
        right_->setSize({right_->getPreferredSize().x, target->size().y - (top_->size().y + bottom_->size().y)});
    }

    left_->setOrigin({target->origin().x, target->origin().y + bottom_->size().y});
    right_->setOrigin({(target->origin().x + target->size().x) - right_->size().x, target->origin().y + bottom_->size().y});

    center_->setSize({target->size().x - (left_->size().x+ right_->size().x), target->size().y - (top_->size().y + bottom_->size().y)});
    center_->setOrigin({left_->origin().x + left_->size().x, left_->origin().y});


}
