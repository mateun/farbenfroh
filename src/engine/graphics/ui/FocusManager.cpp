//
// Created by mgrus on 29.03.2025.
//

#include "FocusManager.h"


#include <windowsx.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/RawWin32Message.h>

#include "MessageHandleResult.h"

FocusManager::FocusManager() {
    printf("in ctr\n");
}

void FocusManager::update() {
    // Find the first matching widget
}

std::shared_ptr<Widget> FocusManager::getFocusedWidget() {
    return previous_focus_widget_;
}

/**
 * We focus on mouse click messages to decide on focus.
 * @param msgs The incoming raw Windows messages from the last frame.
 */
 void FocusManager::onFrameMessages(const std::vector<RawWin32Message>& msgs) {

    for (auto m : msgs) {
     switch (m.message) {
         case WM_MOUSEMOVE: {

             mouse_x = GET_X_LPARAM(m.lParam);
             mouse_y = getApplication()->scaled_height() - GET_Y_LPARAM(m.lParam);

             auto visitor = HitVisitor();
             visitor.visit(getApplication()->getTopLevelWidget(), mouse_x, mouse_y);
             if (auto highestHitWidget = visitor.getHighestHitWidget()) {
                 if (previous_focus_widget_) {
                     previous_focus_widget_->removeHoverFocus();
                 }
                 highestHitWidget->setHoverFocus(previous_focus_widget_);

                 // Send out focues messages
                 UIMessage msg;
                 msg.type = MessageType::WidgetGainedFocus;
                 msg.focusMessage.widget = highestHitWidget;
                 getApplication()->getTopLevelWidget()->onMessage(msg);
                 getApplication()->getCentralSubMenuManager()->onMessage(msg);
                 msg.type = MessageType::WidgetLostFocus;
                 msg.focusMessage.widget = previous_focus_widget_;
                 getApplication()->getTopLevelWidget()->onMessage(msg);

                 previous_focus_widget_ = highestHitWidget;

             }

         }
     }
    }
}

void HitVisitor::visit(std::shared_ptr<Widget> widget, int mouse_x, int mouse_y) {
    if (!widget->isVisible()) return;
    if (widget->getZValue() > hightest_z_value_) {

        if (widget->checkMouseOver(mouse_x, mouse_y)) {
            current_highest_widget = widget;
            hightest_z_value_ = widget->getZValue();
        }
    }

    // First, check any menubar:

    if (widget->hasMenuBar()) {
        visit((widget->getMenuBar()), mouse_x, mouse_y);
    }

    for (auto& c : widget->children()) {
        visit(c, mouse_x, mouse_y);
    }
}

std::shared_ptr<Widget> HitVisitor::getHighestHitWidget() {
    return current_highest_widget;
}

