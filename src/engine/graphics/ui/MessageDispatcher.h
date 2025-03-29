//
// Created by mgrus on 29.03.2025.
//

#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H
#include <engine/graphics/ui/FrameMessageSubscriber.h>

class FocusManager;
/**
*   This class gathers all system messages, creates UIMessage objects out
*   of them, and sends them to the currently focused Widget.
*
*/
class MessageDispatcher : public FrameMessageSubscriber {

  public:

    /**
    *   We need the FocusManager to tell us which Widget has currently the focus.
    *   This is then the target of our UIMessages.
    *
    */
    MessageDispatcher(FocusManager& focusManager);

    /**
    *   Should be called every frame so the messages flow to the
    *   focused Widget without delay.
    */
    void update();

    /**
    *   The callback through which we retrieve all raw OS (win32) messages per frame.
    */
    void onFrameMessages(const std::vector<RawWin32Message> &msgs) override;


private:
    FocusManager &focus_manager_;
};



#endif //MESSAGEDISPATCHER_H
