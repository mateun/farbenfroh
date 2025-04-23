//
// Created by mgrus on 09.04.2025.
//

#include "EdgeDragger.h"

#include <iostream>
#include <engine/graphics/Application.h>
#include <engine/graphics/Widget.h>

#include "FocusManager.h"
#include "MessageTransformer.h"
#include "UIMessage.h"

EdgeDragger::EdgeDragger(std::shared_ptr<Edge> edge, float zValue): edge_(edge), z_value_(zValue) {

}

// We look for mousemove and mousedown to see if the mouse is over our edge.
// TODO check movement against constraints, otherwise change origin accordingly.
void EdgeDragger::onFrameMessages(const std::vector<RawWin32Message> &msgs) {
    // We only act if we can possible "see" the mouse, and we are not hidden, e.g. by a floating window.
    if (getApplication()->getFocusManager()->getFocusedWidget()) {
        // Check if our z-value is higher than that of the currently focused widget.
        // If not we can never drag.
        if (z_value_ < getApplication()->getFocusManager()->getFocusedWidget()->getZValue()) return;
    }

    for (auto msg : msgs) {
        auto tm = MessageTransformer::transform(msg);
        if (tm.type == MessageType::MouseMove) {
            const auto mx = tm.mouseMoveMessage.x;
            const auto my = tm.mouseMoveMessage.y;
            if (edge_->direction == EdgeDirection::Vertical) {
                if (mx > edge_->origin.x - 5 && mx < edge_->origin.x + 5) {
                    hover_ = true;
                    getApplication()->setSpecialCursor(CursorType::ResizeHorizontal);
                } else {
                    hover_ = false;
                    getApplication()->unsetSpecialCursor();
                }
            } else {
                if (my > edge_->origin.y - 5 && my < edge_->origin.y + 5) {
                    hover_ = true;
                    getApplication()->setSpecialCursor(CursorType::ResizeVertical);
                } else {
                    hover_ = false;
                    getApplication()->unsetSpecialCursor();
                }
            }

            if (dragging_) {
                if (edge_->direction == EdgeDirection::Vertical) {
                    getApplication()->setSpecialCursor(CursorType::ResizeHorizontal);
                    edge_->origin.x = mx;
                } else {
                    getApplication()->setSpecialCursor(CursorType::ResizeVertical);
                    edge_->origin.y = my;
                }
            }

        }

        if (tm.type == MessageType::MouseDown) {
            if (hover_) {
                dragging_ = true;
                getApplication()->setSpecialCursor(CursorType::ResizeHorizontal);
            }
        }

        if (tm.type == MessageType::MouseUp) {
            dragging_ = false;
        }
    }

}

glm::vec2 EdgeDragger::getOrigin() {
    return edge_->origin;
}

bool EdgeDragger::isHovering() {
    return hover_;
}

void EdgeDragger::setZValue(float val) {
    z_value_ = val;
}
