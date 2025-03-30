//
// Created by mgrus on 29.03.2025.
//

#ifndef MESSAGETRANSFORMER_H
#define MESSAGETRANSFORMER_H

struct UIMessage;
struct RawWin32Message;

class MessageTransformer {

  public:
    static UIMessage transform(RawWin32Message message);

};



#endif //MESSAGETRANSFORMER_H
