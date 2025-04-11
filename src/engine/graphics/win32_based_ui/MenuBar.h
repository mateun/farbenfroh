//
// Created by mgrus on 11.04.2025.
//

#ifndef MENUBAR_H
#define MENUBAR_H

#include <memory>
#include <Windows.h>

namespace win32 {
    class WinMenu;

    class WinMenuBar {
      public:
        explicit WinMenuBar(HWND hwnd);
        void addMenu(std::shared_ptr<WinMenu> menu);

    private:
        HMENU h_menu_bar_;
        HWND hwnd_;
    };

} // win32

#endif //MENUBAR_H
