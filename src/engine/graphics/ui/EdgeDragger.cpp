//
// Created by mgrus on 09.04.2025.
//

#include "EdgeDragger.h"

#include <engine/graphics/Application.h>
#include <engine/graphics/Widget.h>

#include "FocusManager.h"
#include "MessageTransformer.h"
#include "UIMessage.h"

EdgeDragger::EdgeDragger(Edge edge, float zValue): edge_(edge), zValue_(zValue) {
    getApplication()->addMessageSubscriber(shared_from_this());
}

// We look for mousemove and mousedown to see if the mouse is over our edge.
// TODO check movement against constraints, otherwise change origin accordingly.
void EdgeDragger::onFrameMessages(const std::vector<RawWin32Message> &msgs) {

    // We only act if we can possible "see" the mouse, and we are not hidden, e.g. by a floating window.
    if (zValue_ < getApplication()->getFocusManager()->getFocusedWidget()->getZValue()) return;

    for (auto msg : msgs) {
        auto tm = MessageTransformer::transform(msg);
        if (tm.type == MessageType::MouseMove) {
            auto mx = tm.mouseMoveMessage.x;
            auto my = tm.mouseMoveMessage.y;
            if (edge_.direction == EdgeDirection::Vertical) {
                if (mx > edge_.origin.x - 3 && mx < edge_.origin.x + 3) {
                    hover_ = true;
                }
            }
        }
    }



}

glm::vec2 EdgeDragger::getOrigin() {
    return edge_.origin;
}
