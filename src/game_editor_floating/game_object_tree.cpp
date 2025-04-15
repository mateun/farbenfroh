//
// Created by mgrus on 14.04.2025.
//

#define UNICODE
#include <Windows.h>

#define ID_MENU_NEW_CUBE         100
#define ID_MENU_NEW_SPHERE       101
#define ID_MENU_NEW_CAPSULE      102
#define ID_MENU_NEW_TERRAIN      103

static void createObjectTreeMenu(HWND hwnd) {
    auto mainMenu = CreateMenu();
    auto menuGameObject = CreatePopupMenu();
    auto menuGameObjectNew = CreatePopupMenu();
    AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)menuGameObject, L"&Game Object");
    AppendMenu(menuGameObject, MF_POPUP, (UINT_PTR) menuGameObjectNew, L"&New");
    AppendMenu(menuGameObjectNew, MF_STRING, ID_MENU_NEW_CUBE, L"&Cube Mesh");
    AppendMenu(menuGameObjectNew, MF_STRING, ID_MENU_NEW_SPHERE, L"&Sphere Mesh");
    AppendMenu(menuGameObjectNew, MF_STRING, ID_MENU_NEW_CAPSULE, L"&Capsule Mesh");
    AppendMenu(menuGameObjectNew, MF_STRING, ID_MENU_NEW_TERRAIN, L"&Terrain");

    // auto windowMenu = CreatePopupMenu();
    // AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)windowMenu, L"&Window");
    // AppendMenu(windowMenu, MF_STRING, ID_MENU_WINDOW_CONSOLE, L"Console");

    SetMenu(hwnd, mainMenu);
}

static LRESULT CALLBACK GameObjectTreeProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    switch (msg)
    {

        case WM_CREATE: {
            createObjectTreeMenu(hwnd);
            break;

        }

        case WM_COMMAND: {

            switch (w) {
//                case ID_MENU_WINDOW_CONSOLE: showConsoleWindow();break;
//                case ID_MENU_NEW_GAME: createNewGameDialog(g_hinstance, g_mainHwnd);break;
            }
        }


        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            // HDC hdc = BeginPaint(hwnd, &ps);
            // SelectObject(hdc, gFont);
            //
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_MOVE: {
            RECT windowRect;
//            if(GetWindowRect(g_mainHwnd, &windowRect))
//            {
//                // The window's position is given by the left and top of the RECT.
//                int x = windowRect.left;
//                int y = windowRect.top;
//
//                // Calculate width and height.
//                int width = windowRect.right - windowRect.left;
//                int height = windowRect.bottom - windowRect.top;
//
//                SetWindowPos(g_helpHwnd, NULL, x + width + 10, y, 400, 200, 0);
//
//            }
            break;
        }

        // case WM_TIMER:
        //     g_cursor_visible = !g_cursor_visible;
        //     InvalidateRect(hwnd, nullptr, FALSE);
        //     return 0;


    }
    return DefWindowProc(hwnd, msg, w, l);
}

HWND createGameObjectTreeWindow(HWND parentWindow, HINSTANCE hInstance) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = GameObjectTreeProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GameObjectTreeWindowClass";
    wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(15, 15, 15)));
    RegisterClass(&wc);

    RECT windowRect;
    if(GetWindowRect(parentWindow, &windowRect))
    {
        // The window's position is given by the left and top of the RECT.
        int x = windowRect.left;
        int y = windowRect.top;

        // Calculate width and height.
        int width = windowRect.right - windowRect.left;
        int height = windowRect.bottom - windowRect.top;

        auto win = CreateWindowEx(0, L"GameObjectTreeWindowClass", L"GameObjectTree", WS_OVERLAPPED | WS_VISIBLE,
        x - 408 , y, 400, height, parentWindow, nullptr, hInstance, nullptr);
        return win;

    }


}
