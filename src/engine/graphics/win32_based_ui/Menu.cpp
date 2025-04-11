//
// Created by mgrus on 11.04.2025.
//

#include <Windows.h>
#include "Menu.h"

#include <string>

win32::WinMenu::WinMenu(HWND hwnd, const std::string &text): text_(text), hwnd_(hwnd) {
    h_menu_ = CreatePopupMenu();
    SetMenu(hwnd, h_menu_);
}

void win32::WinMenu::addItem(const std::string &text, const int commandId) {
    AppendMenu(h_menu_, MF_STRING, commandId, text.c_str());
    DrawMenuBar(hwnd_);
}

HMENU win32::WinMenu::handle() const {
    return h_menu_;
}

std::string win32::WinMenu::text() const {
    return text_;
}
