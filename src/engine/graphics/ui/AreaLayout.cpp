//
// Created by mgrus on 30.03.2025.
//

#include <engine/graphics/PlanePivot.h>
#include <engine/graphics/Widget.h>

#include "EdgeDragger.h"
#include "EdgePainter.h"

AreaLayout::AreaLayout() : AreaLayout(nullptr, nullptr, nullptr, nullptr, nullptr) {
}

std::shared_ptr<Edge> AreaLayout::createEdge(LayoutPosition layoutPosition) {
    auto edge = std::make_shared<Edge>();
    switch (layoutPosition) {
        case LayoutPosition::Top: {
            edge->direction = EdgeDirection::Horizontal;
            edge->origin = top_->origin();
            edge->size = top_->size().x;
            break;
        }
        case LayoutPosition::Bottom: {
            edge->direction = EdgeDirection::Horizontal;
            edge->origin = bottom_->origin();
            edge->size = bottom_->size().x;
            break;
        }
        case LayoutPosition::Left: {
            edge->direction = EdgeDirection::Vertical;
            edge->origin = left_->origin() + glm::vec2(left_->size().x, 0);
            edge->size = left_->size().y;
            break;
        }
        case LayoutPosition::Right: {
            edge->direction = EdgeDirection::Vertical;
            edge->origin = right_->origin();
            edge->size = right_->size().y;
            break;
        }
    }

    return edge;
}

AreaLayout::AreaLayout(std::shared_ptr<Widget> top, std::shared_ptr<Widget> bottom, std::shared_ptr<Widget> left,
                       std::shared_ptr<Widget> right, std::shared_ptr<Widget> center): top_(std::move(top)), bottom_(std::move(bottom)), left_(std::move(left)),
                                                                                       right_(std::move(right)), center_(std::move(center)) {
    createPlaceholdersForNullParts();


}

void AreaLayout::createPlaceholdersForNullParts() {
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

void AreaLayout::apply(Widget *target) {
    // We assume our edge draggers are already setup
    // and we can ask them to provide any updated edge positions
    // in case the user dragged them.
    if (!first_time_) {
        edge_dragger_right_->getOrigin();
    }

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

    if (edge_painter_) {
        edge_painter_->draw(target->getZValue() + 0.01f);
    }

    // Setup our edge draggers if this is the first time application of this layout.
    // We now know the initial sizes of all areas and can initialize the respective edges correctly:
    if (first_time_) {
        first_time_ = false;
        auto edge_top = createEdge(LayoutPosition::Top);
        auto edge_bottom = createEdge(LayoutPosition::Bottom);
        auto edge_left = createEdge(LayoutPosition::Left);
        auto edge_right = createEdge(LayoutPosition::Right);
        edge_dragger_top_ = std::make_shared<EdgeDragger>(edge_top, target->getZValue());
        edge_dragger_bottom_ = std::make_shared<EdgeDragger>(edge_bottom, target->getZValue());
        edge_dragger_left_ = std::make_shared<EdgeDragger>(edge_left, target->getZValue());
        edge_dragger_right_ = std::make_shared<EdgeDragger>(edge_right, target->getZValue());

        edge_painter_ = std::make_shared<EdgePainter>();
        edge_painter_->addEdge(edge_top);
        edge_painter_->addEdge(edge_bottom);
        edge_painter_->addEdge(edge_left);
        edge_painter_->addEdge(edge_right);
    }

}
