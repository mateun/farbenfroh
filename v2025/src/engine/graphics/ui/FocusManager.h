//
// Created by mgrus on 29.03.2025.
//

#ifndef FOCUSMANAGER_H
#define FOCUSMANAGER_H

#include <memory>
#include <engine/graphics/ui/FrameMessageSubscriber.h>

class Widget;

/**
* This class gathers mouse and keyboard events
* and derives from those the currently focused widget.
* E.g. by tabbing around, the focus might change round robin.
* Or when the user moves the mouse and then clicks while over given widget,
* this widget gains focus.
* As soon as the widget was identified, its "setFocus" method is called.
* The former focused widget gets the "removeFocus" method invoked.
* The FocusManager can also report back the currently focused widget, e.g. for the
* MessageDispatcher to send the messages to the correct widget.
*/
class FocusManager : public FrameMessageSubscriber {

  public:
    FocusManager();

    /**
    * Return a pointer to a Widget which has currently the focus.
    */
    std::shared_ptr<Widget> getFocusedWidget();

    /**
    * Here we are called by the OS with all frame messages.
    */
    void onFrameMessages(const std::vector<RawWin32Message>& msgs) override;

    int mouse_x = 0, mouse_y = 0;
    std::shared_ptr<Widget> previous_focus_widget_;
};

class HitVisitor {
public:
    void visit(std::shared_ptr<Widget> widget, int mouse_x, int mouse_y);

    std::shared_ptr<Widget> getHighestHitWidget();


private:
    std::shared_ptr<Widget> current_highest_widget;
    float hightest_z_value_ = std::numeric_limits<float>::lowest();
};



#endif //FOCUSMANAGER_H
