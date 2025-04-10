//
// Created by mgrus on 29.03.2025.
//

#include "FocusManager.h"


#include <iostream>
#include <windowsx.h>
#include <engine/graphics/Application.h>
#include <engine/graphics/RawWin32Message.h>

#include "FloatingWindow.h"
#include "MessageHandleResult.h"
#include "MessageTransformer.h"
#include <engine/graphics/ui/UIMessage.h>

FocusManager::FocusManager() {
    printf("in ctr\n");
}

std::shared_ptr<Widget> FocusManager::getFocusedWidget() {
    return previous_focus_widget_;
}

/**
 * We focus on mouse mouse move and click messages to decide on focus.
 * TODO maybe allow for tab cycling later.
 * @param msgs The incoming raw Windows messages from the last frame.
 */
 void FocusManager::onFrameMessages(const std::vector<RawWin32Message>& msgs) {

    for (auto m : msgs) {
        auto uim = MessageTransformer::transform(m);
     switch (uim.type) {
         case MessageType::MouseMove: {

             mouse_x = uim.mouseMoveMessage.x;
             mouse_y = uim.mouseMoveMessage.y;


             // First visit every floating window, they are always first in z-order.
             {
                 bool foundFocusAmongFloatingWindows = false;
                 float floatingWindowHightestZ = std::numeric_limits<float>::lowest();
                 std::shared_ptr<FloatingWindow> highestFloatingWindow;
                 for (auto floatingWindow : getApplication()->getFloatingWindows()) {
                    if (!floatingWindow->isVisible()) continue;

                    if (floatingWindow->checkMouseOver(mouse_x, mouse_y)) {
                        if (floatingWindow->getZValue() > floatingWindowHightestZ) {
                             floatingWindowHightestZ = floatingWindow->getZValue();
                             highestFloatingWindow = floatingWindow;
                         }

                     }

                 }

                 if (highestFloatingWindow) {

                     highestFloatingWindow->setHoverFocus(nullptr);
                     foundFocusAmongFloatingWindows = true;
                     auto visitor = HitVisitor();
                     visitor.visit(highestFloatingWindow, mouse_x, mouse_y);
                     if (auto hitWindowWidget = visitor.getHighestHitWidget()) {
                         if (previous_focus_widget_) {
                             previous_focus_widget_->removeHoverFocus();
                         }
                         std::cout << "focused widget in window: " << highestFloatingWindow->getId() << "->" << hitWindowWidget->getId() << std::endl;
                         hitWindowWidget->setHoverFocus(previous_focus_widget_);
                         previous_focus_widget_ = hitWindowWidget;
                     }
                 }

                 // Swallow this message in case we hit a floating window:
                 if (foundFocusAmongFloatingWindows) continue;
             }

             // Next we visit the menubar
             auto menuBar = getApplication()->getMenuBar();
             if (menuBar) {
                 auto visitor = HitVisitor();
                 visitor.visit(menuBar, mouse_x, mouse_y);
                 if (auto highestHitWidget = visitor.getHighestHitWidget()) {
                     if (previous_focus_widget_) {
                         previous_focus_widget_->removeHoverFocus();
                     }
                     std::cout << "focused widget in window: " << highestHitWidget->getId() << "->" << highestHitWidget->getId() << std::endl;
                     highestHitWidget->setHoverFocus(previous_focus_widget_);
                     previous_focus_widget_ = highestHitWidget;

                     UIMessage msg;
                     msg.num = m.num;
                     msg.type = MessageType::WidgetGainedFocus;
                     msg.focusMessage.widget = highestHitWidget;
                     msg.sender = "FocusManager";

                     getApplication()->getCentralSubMenuManager()->onMessage(msg);

                     continue;
                 }
             }

             // Next pass the message on to the toplevel widget of the application:
             {
                 auto visitor = HitVisitor();
                 visitor.visit(getApplication()->getTopLevelWidget(), mouse_x, mouse_y);
                 if (auto highestHitWidget = visitor.getHighestHitWidget()) {
                     if (previous_focus_widget_) {
                         previous_focus_widget_->removeHoverFocus();
                     }
                     highestHitWidget->setHoverFocus(previous_focus_widget_);

                     std::cout << "focused widget: " << highestHitWidget->getId() << " z-value: " << highestHitWidget->getZValue() << std::endl;

                     // Send out dedicated focus messages
                     // First, a GainedFocus message:
                     UIMessage msg;
                     msg.num = m.num;
                     msg.type = MessageType::WidgetGainedFocus;
                     msg.focusMessage.widget = highestHitWidget;
                     msg.sender = "FocusManager";

                     getApplication()->getCentralSubMenuManager()->onMessage(msg);

                     // Then a LostFocus message for the prev. holding widget:
                     // msg.num = m.num;
                     // msg.type = MessageType::WidgetLostFocus;
                     // msg.focusMessage.widget = previous_focus_widget_;

                     previous_focus_widget_ = highestHitWidget;

                 }
             }

         }
     }
    }
}

void HitVisitor::visit(std::shared_ptr<Widget> widget, int mouse_x, int mouse_y) {
    if (!widget->isVisible()) return;
    if (widget->getZValue() > hightest_z_value_) {

        std::cout << "checking hover for widget: " << widget->getId() << "@ origin: "
                    << std::to_string(widget-> origin().x) << "/"
                    << std::to_string(widget-> origin().y)
                    << " @ size: " << widget->size().x << "/"
                    << widget->size().y
                    << std::endl;

        if (widget->checkMouseOver(mouse_x, mouse_y)) {
            current_highest_widget = widget;
            hightest_z_value_ = widget->getZValue();
            std::cout << "check succeeded!" << std::endl;

        } else {
            std::cout << "check failed." << std::endl;
        }
    }

    // First, visit all menubars of this widget:
    if (widget->hasMenuBar()) {
        visit((widget->getMenuBar()), mouse_x, mouse_y);
    }

    // Then the children of this widget:
    for (auto& c : widget->children()) {
        visit(c, mouse_x, mouse_y);
    }
}

std::shared_ptr<Widget> HitVisitor::getHighestHitWidget() {
    return current_highest_widget;
}

