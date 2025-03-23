//
// Created by mgrus on 23.03.2025.
//

#include "Application.h"

// This function must be provided by any Application implementor.
extern std::unique_ptr<Application> getApplication();

Application::Application(int w, int h, bool fullscreen) : width(w), height(h), fullscreen(fullscreen){

}

Application::~Application() {
}

void Application::initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpstr, int nShowCmd) {

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    auto performance_frequency = freq.QuadPart;


#ifdef _WITH_CONSOLE
    // Allocate a dedicated console and redirect stdout.
    // Should only be done for debugging purposes, not in the final production build.
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
#endif

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
    wc.lpfnWndProc = Application::AppWindowProc;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        exit(1);
    }

    RECT corrRect = {0, 0, width, height};
    AdjustWindowRect(&corrRect, WS_OVERLAPPEDWINDOW, false);

    auto winWidthHalf = (corrRect.right  - corrRect.left) / 2;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);


    _window = CreateWindow(
			 g_szClassName,
             "Application",
			 //WS_POPUP,
			 WS_OVERLAPPEDWINDOW,
			 screenWidth/2 - winWidthHalf, 0, corrRect.right - corrRect.left, corrRect.bottom - corrRect.top,
			 NULL, NULL, hInstance, this);



    if (_window == NULL) {
        MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    }

    ShowWindow(_window, SW_NORMAL);
    UpdateWindow(_window);
    // HDC hdc = GetDC(hwnd);
    // registerRawInput(hwnd);


}

int Application::run() {
	mainLoop();
	return 0;
}


#ifdef USE_WIN32_APP_FRAMEWORK
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nShowCmd) {

	std::unique_ptr<Application> app = getApplication();
	app->initialize(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
	return app->run();

}
#endif


void Application::mainLoop() {

    //initXInput();

	bool running = true;

	MSG msg;
	while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

	    Sleep(1);

	}

}

LRESULT Application::AppWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Application* pThis = nullptr;
    if (message == WM_NCCREATE) {
        // On WM_NCCREATE, extract the 'this' pointer from the CREATESTRUCT
        LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<Application*>(pcs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

#define ID_LEFT_CHILD  101
#define ID_RIGHT_CHILD 102

    static HWND hwndLeft = nullptr;
    static HWND hwndRight = nullptr;

    switch (message) {

        case WM_CREATE: {
            // Create the left child window.
            hwndLeft = CreateWindowEx(
                0,                        // Extended style
                "STATIC",                // Predefined class for a static control
                "Left Pane",             // Window text (optional)
                WS_CHILD | WS_VISIBLE,    // Child window style
                0, 0,                     // x, y position (will adjust in WM_SIZE)
                100, 100,                 // Width, height (temporary values)
                hWnd,                     // Parent window handle
                (HMENU)ID_LEFT_CHILD,     // Child window ID
                ((LPCREATESTRUCT)lParam)->hInstance,
                nullptr                   // Additional parameters
            );

            // Create the right child window.
            hwndRight = CreateWindowEx(
                0,                        // Extended style
                "STATIC",                // Predefined class for a static control
                "Right Pane",            // Window text (optional)
                WS_CHILD | WS_VISIBLE,    // Child window style
                100, 0,                   // x, y position (temporary, will update)
                100, 100,                 // Width, height (temporary)
                hWnd,                     // Parent window handle
                (HMENU)ID_RIGHT_CHILD,    // Child window ID
                ((LPCREATESTRUCT)lParam)->hInstance,
                nullptr                   // Additional parameters
            );
            return 0;
        }
        case WM_SIZE: {
            // Get the new dimensions of the main window's client area.
            int clientWidth  = LOWORD(lParam);
            int clientHeight = HIWORD(lParam);
            // Define a fixed width for the left pane, or calculate dynamically.
            int leftPaneWidth = clientWidth / 3;
            // for now override left width:
            leftPaneWidth = 400;

            // Position the left child window.
            MoveWindow(hwndLeft, 0, 0, leftPaneWidth, clientHeight, TRUE);

            // Position the right child window to occupy the rest of the space.
            MoveWindow(hwndRight, leftPaneWidth +2, 0, clientWidth - leftPaneWidth - 2, clientHeight, TRUE);
            return 0;
        }

        case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
        // For raw mouse input
        case WM_INPUT:
        {
            // UINT dwSize;
            // GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
            // LPBYTE lpb = new BYTE[dwSize];
            // if (lpb == NULL) {
            //     return 0;
            // }
            //
            // if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
            //     delete[] lpb;
            //     return 0;
            // }
            //
            // RAWINPUT* raw = (RAWINPUT*)lpb;
            // if (raw->header.dwType == RIM_TYPEMOUSE) {
            //
            //     mouse_rel_x = raw->data.mouse.lLastX;
            //     mouse_rel_y = raw->data.mouse.lLastY;
            //
            //
            // }
            //
            // delete[] lpb;
            // return 0;
        }


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
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            if (abs(pt.x - 400) <= 4) {
                SetCursor(LoadCursor(NULL, IDC_SIZEWE));
                return TRUE;
            }

            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return TRUE;


        }
        default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


