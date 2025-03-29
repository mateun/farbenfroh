//
// Created by mgrus on 29.03.2025.
//

#ifndef RAWWIN32MESSAGE_H
#define RAWWIN32MESSAGE_H

#include <windows.h>

/**
* This captures the important parts of a raw win32 message as received by wndproc.
*/
struct RawWin32Message {
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
};

#endif //RAWWIN32MESSAGE_H
