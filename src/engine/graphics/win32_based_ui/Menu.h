//
// Created by mgrus on 11.04.2025.
//

#ifndef MENU_H
#define MENU_H
#include <string>

namespace win32 {

class WinMenu {
  public:
    WinMenu(HWND hwnd, const std::string& text);
    void addItem(const std::string& text, int commandId);
    HMENU handle() const;
    std::string text() const;

private:
    HMENU h_menu_;
    std::string text_;
    HWND hwnd_;
};

} // win32

#endif //MENU_H
