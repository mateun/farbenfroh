//
// Created by mgrus on 29.03.2025.
//

#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H
#include <memory>
#include <engine/graphics/Widget.h>
#include <engine/graphics/ui/FrameMessageSubscriber.h>

class FocusManager;


class SimpleMessageDispatcher : public FrameMessageSubscriber {
public:
    void onFrameMessages(const std::vector<RawWin32Message> &msgs) override;

};

/**
*   This class gathers all system messages, creates UIMessage objects out
*   of them, and sends them to the currently focused Widget.
*
*/
class FocusBasedMessageDispatcher : public FrameMessageSubscriber {

  public:


    /**
    *   We need the FocusManager to tell us which Widget has currently the focus.
    *   This is then the target of our UIMessages.
    *
    */
    FocusBasedMessageDispatcher(FocusManager& focusManager);

    /**
    *   The callback through which we retrieve all raw OS (win32) messages per frame.
    */
    void onFrameMessages(const std::vector<RawWin32Message> &msgs) override;


private:
    FocusManager &focus_manager_;
    uint64_t msg_number_ = 0;
};



#endif //MESSAGEDISPATCHER_H
