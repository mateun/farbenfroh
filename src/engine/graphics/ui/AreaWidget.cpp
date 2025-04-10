//
// Created by mgrus on 10.04.2025.
//

#include "AreaWidget.h"
#include "EdgeDragger.h"

AreaWidget::AreaWidget() : AreaWidget(nullptr, nullptr, nullptr, nullptr, nullptr) {

}

std::shared_ptr<Edge> AreaWidget::createEdge(AreaLayoutPosition layoutPosition) {
    auto edge = std::make_shared<Edge>();
    switch (layoutPosition) {
        case AreaLayoutPosition::Top: {
            edge->direction = EdgeDirection::Horizontal;
            edge->origin = top_->origin();
            edge->size = top_->size().x;
            break;
        }
        case AreaLayoutPosition::Bottom: {
            edge->direction = EdgeDirection::Horizontal;
            edge->origin = bottom_->origin() + glm::vec2(left_->size().x, bottom_->size().y);
            edge->size = bottom_->size().x;
            break;
        }
        case AreaLayoutPosition::Left: {
            edge->direction = EdgeDirection::Vertical;
            edge->origin = left_->origin() + glm::vec2(left_->size().x, 0);
            edge->size = left_->size().y;
            break;
        }
        case AreaLayoutPosition::Right: {
            edge->direction = EdgeDirection::Vertical;
            edge->origin = right_->origin();
            edge->size = right_->size().y;
            break;
        }
    }

    return edge;
}

void AreaWidget::createPlaceholdersForNullParts() {
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

AreaWidget::AreaWidget(std::shared_ptr<Widget> top, std::shared_ptr<Widget> bottom, std::shared_ptr<Widget> left,
                       std::shared_ptr<Widget> right, std::shared_ptr<Widget> center): top_((top)), bottom_((bottom)),
                                                                                       left_((left)), center_((center)),
                                                                                       right_((right)) {

    createPlaceholdersForNullParts();
    if (size().x == 0 || size().y == 0) {
        setSize({getApplication()->width(), getApplication()->height()});
    }
    layout_ = std::make_shared<AreaLayout>(top_, bottom_, left_, right_, center_);
    layout_->apply(this);

    edge_top = createEdge(AreaLayoutPosition::Top);
    edge_bottom = createEdge(AreaLayoutPosition::Bottom);
    edge_left = createEdge(AreaLayoutPosition::Left);
    edge_right = createEdge(AreaLayoutPosition::Right);



    // TODO the current zValue for sure is bad - we need to update the zvalue at drawing time.
    edge_dragger_top_ = std::make_shared<EdgeDragger>(edge_top, getZValue());
    edge_dragger_bottom_ = std::make_shared<EdgeDragger>(edge_bottom, getZValue());
    edge_dragger_left_ = std::make_shared<EdgeDragger>(edge_left, getZValue());
    edge_dragger_right_ = std::make_shared<EdgeDragger>(edge_right, getZValue());

    getApplication()->addMessageSubscriber(edge_dragger_top_);
    getApplication()->addMessageSubscriber(edge_dragger_bottom_);
    getApplication()->addMessageSubscriber(edge_dragger_left_);
    getApplication()->addMessageSubscriber(edge_dragger_right_);

    edge_painter_ = std::make_shared<EdgePainter>();
    edge_painter_->addEdge(edge_top);
    edge_painter_->addEdge(edge_bottom);
    edge_painter_->addEdge(edge_left);
    edge_painter_->addEdge(edge_right);

}

void AreaWidget::draw(float depth) {
    edge_dragger_top_->setZValue(depth);

    auto areaLayout = std::dynamic_pointer_cast<AreaLayout>(layout_);
    areaLayout->setEdge(edge_left, AreaLayoutPosition::Left);
    areaLayout->setEdge(edge_right, AreaLayoutPosition::Right);
    areaLayout->setEdge(edge_top, AreaLayoutPosition::Top);
    areaLayout->setEdge(edge_bottom, AreaLayoutPosition::Bottom);

    layout_->apply(shared_from_this().get());
    top_->draw(depth);
    left_->draw(depth);
    right_->draw(depth);
    bottom_->draw(depth);
    center_->draw(depth);

    edge_painter_->draw(depth + 0.01f);



}
