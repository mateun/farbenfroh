//
// Created by mgrus on 14.04.2025.
//

#define UNICODE
#include <Windows.h>



LRESULT CALLBACK GameObjectTreeProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    switch (msg)
    {

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
