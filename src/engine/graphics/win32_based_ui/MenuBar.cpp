//
// Created by mgrus on 11.04.2025.
//

#include <Windows.h>
#include <engine/graphics/win32_based_ui/MenuBar.h>
#include <engine/graphics/win32_based_ui/Menu.h>


win32::WinMenuBar::WinMenuBar(HWND hwnd): hwnd_(hwnd) {
    h_menu_bar_ = CreateMenu();
    SetMenu(hwnd, h_menu_bar_);
}

void win32::WinMenuBar::addMenu(std::shared_ptr<WinMenu> menu) {
    AppendMenu(h_menu_bar_, MF_POPUP, (UINT_PTR)menu->handle(), menu->text().c_str());
    DrawMenuBar(hwnd_);

}

