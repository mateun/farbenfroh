//
// Created by mgrus on 23.04.2025.
//

#include <engine.h>
#include <windowsx.h>

ENGINE_API WPARAM lastKeyPress = 0;
int mouse_x = 0;
int mouse_y = 0;
int mouse_rel_x = 0;
int mouse_rel_y = 0;
bool useMouse = true;
static int window_height = -1;
static int window_width = -1;


static LRESULT WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {


    switch (message) {

        case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
        // For raw mouse input
        case WM_INPUT:
        {
            UINT dwSize;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
            LPBYTE lpb = new BYTE[dwSize];
            if (lpb == NULL) {
                return 0;
            }

            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
                delete[] lpb;
                return 0;
            }

            RAWINPUT* raw = (RAWINPUT*)lpb;
            if (raw->header.dwType == RIM_TYPEMOUSE) {

                mouse_rel_x = raw->data.mouse.lLastX;
                mouse_rel_y = raw->data.mouse.lLastY;


            }

            delete[] lpb;
            return 0;
        }

        case WM_MOUSEMOVE:
            if (useMouse) {
                mouse_x = GET_X_LPARAM(lParam);
                mouse_y = GET_Y_LPARAM(lParam);
            }

            break;

        case WM_SIZE:
            break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            lastKeyPress = wParam;
            break;
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
            ShowCursor(TRUE);
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;
        }

         case WM_SETCURSOR:{
            // lParam's low-order word indicates the hit-test result.
            WORD hitTest = LOWORD(lParam);
            if (hitTest == HTCLIENT) {
                // In the client area, explicitly set the cursor to the standard arrow - unless the application
                // tells us to allow the cursor override:
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE; // Message handled.
            }

            // For non-client areas (like the edges), let Windows handle it.
            return DefWindowProc(hWnd, message, wParam, lParam);

        }
        default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HWND ENGINE_API create_window(int w, int h, bool fullscreen, HINSTANCE hInstance, const std::string& window_title) {

    window_height = h;
    window_width = w;

    // Window setup
    const char g_szClassName[] = "winClass";
    WNDCLASSEX wc;
    wc.lpszMenuName = NULL;
    wc.hInstance = hInstance;

    wc.lpszClassName = g_szClassName;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbClsExtra = wc.cbWndExtra = 0;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.hbrBackground = CreateSolidBrush(RGB(64, 64, 64));
    //wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hCursor = NULL;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.lpfnWndProc = WinProc;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        exit(1);
    }

    RECT corrRect = {0, 0, w, h};
    AdjustWindowRect(&corrRect, WS_OVERLAPPEDWINDOW, false);

    auto winWidthHalf = (corrRect.right  - corrRect.left) / 2;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);


    HWND win = CreateWindow(
             g_szClassName,
             window_title.c_str(),
             //WS_POPUP,
             WS_OVERLAPPEDWINDOW,
             screenWidth/2 - winWidthHalf, 0, corrRect.right - corrRect.left, corrRect.bottom - corrRect.top,
             NULL, NULL, hInstance, 0);



    if (win == NULL) {
        MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    }

    ShowWindow(win, SW_NORMAL);
    UpdateWindow(win);
    return win;

}

bool ENGINE_API poll_window(HWND window) {

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) > 0) {
        if (msg.message == WM_QUIT) {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}
