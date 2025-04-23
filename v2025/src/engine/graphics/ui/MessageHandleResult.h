//
// Created by mgrus on 29.03.2025.
//

#ifndef MESSAGEHANDLERESULT_H
#define MESSAGEHANDLERESULT_H

#include <string>

/**
* Defines the result of message handling, including success/failure, handled it etc.
*/
struct MessageHandleResult {

  bool success = false;
  std::string errorMessage;
  bool wasHandled = false;


};

#endif //MESSAGEHANDLERESULT_H
