//
// Created by mgrus on 29.03.2025.
//

#ifndef FRAMEMESSAGESUBSCRIBER_H
#define FRAMEMESSAGESUBSCRIBER_H

#include <vector>

struct RawWin32Message;
/**
* Receives a list of raw windows messages each frame.
*/
class FrameMessageSubscriber {
public:
  virtual void onFrameMessages(const std::vector<RawWin32Message>& msgs) = 0;

};



#endif //FRAMEMESSAGESUBSCRIBER_H
