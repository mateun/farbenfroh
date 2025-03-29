//
// Created by mgrus on 29.03.2025.
//

#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H
#include <engine/graphics/FrameMessageSubscriber.h>
#include <engine/graphics/FrameMessageSubscriber.h>


/**
* This class gathers all system messages, creates UIMessage objects out
* of them, and sends them to the currently focused Widget.
*
*/
class MessageDispatcher : public FrameMessageSubscriber{

  public:
    // Should be called every frame so the messages flow to the
    // focused Widget without delay.
    void update();

    /**
    * So we retrieve all raw OS (win32) messages per frame.
    */
    void onFrameMessages(const std::vector<MSG> &msgs) override;
};



#endif //MESSAGEDISPATCHER_H
