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
  UINT message = 0;
  WPARAM wParam = 0;
  LPARAM lParam = 0;
  uint64_t num = 0;
};

#endif //RAWWIN32MESSAGE_H
