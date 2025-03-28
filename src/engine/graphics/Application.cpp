//
// Created by mgrus on 23.03.2025.
//

#include "Application.h"

#include <ranges>

#include <engine/profiling/PerformanceTimer.h>
#include <engine/graphics/MeshDrawData.h>
#include <GL/glew.h>
#include "RenderBackend.h"
#include "Widget.h"
#include <engine/game/Timing.h>
#include <engine/input/Input.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <iostream>
#include <ostream>
#include <engine/graphics/stb_truetype.h>

#include "Renderer.h"

// This function must be provided by any Application implementor.
extern std::shared_ptr<Application> getApplication();

Application::Application(int w, int h, bool fullscreen) : width(w), height(h), fullscreen(fullscreen), scaled_width_(width), scaled_height_(height) {

}

Application::~Application() {
}


bool Application::changeResolution(int width, int height, int refreshRate, const std::string& deviceName, bool goFullscreen) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    auto dpi = GetDpiForWindow(_window);
    auto dpiScaleFactor = static_cast<float>(dpi) / 96.0f;
    scaled_width_ = width / dpiScaleFactor;
    scaled_height_ = height / dpiScaleFactor;

    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmDisplayFrequency = refreshRate;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

    // Apply the new display settings
    if (goFullscreen) {
        if (ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            MessageBox(nullptr, "Failed to change display settings", "Error", MB_OK | MB_ICONERROR);
            return false;
        }

        // Remove window borders and make it topmost
        SetWindowLong(_window, GWL_STYLE, WS_POPUP);
        SetWindowPos(_window, HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED);
        ShowWindow(_window, SW_MAXIMIZE);

        // Update our internal window size variables:

        fullscreen = true;

        return true;

    }
    // TODO: currently nothing done if none fullscreen change. then we assume we are developing
    //  and just stay with the bordered window.
    return true;

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
    hdc = GetDC(_window);
    render_backend_ = std::make_unique<RenderBackend>(RenderBackendType::OpenGL, hdc, _window, width, height);
    // registerRawInput(hwnd);



}

int Application::run() {
    onCreated();
	mainLoop();
	return 0;
}

RenderBackend * Application::getRenderBackend() const {
    return render_backend_.get();
}

int Application::scaled_width() {
    return scaled_width_;
}

int Application::scaled_height() {
    return scaled_height_;
}

#ifdef USE_WIN32_APP_FRAMEWORK
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nShowCmd) {

	std::shared_ptr<Application> app = getApplication();
	app->initialize(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
	return app->run();

}
#endif


void Application::setTopLevelWidget(const std::shared_ptr<Widget>& widget) {
    topLevelWidget = widget;

    // The top level widget is always the full application window size.
    // Further down the hierarchy, e.g. splitted windows have smaller sizes.
    topLevelWidget->setSize({width, height});
}

void Application::mainLoop() {

    //initXInput();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	bool running = true;

    PerformanceTimer performance_timer;
	MSG msg;
	while (running) {
	    performance_timer.start();
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }


	    if (topLevelWidget) {
	        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	        // We provide an ortho camera which represents the application window dimensions.
	        //render_backend_->setViewport(0,0, scaled_width(), scaled_height());
	        //auto camera = render_backend_->getOrthoCameraForViewport(0, 0, scaled_width(), scaled_height());
	        topLevelWidget->draw();
            Renderer::submitDeferredWidgetCalls();
	        // // Finally present to the main framebuffer.
	         SwapBuffers(hdc);
	    }
	    performance_timer.stop();
	    Timing::frameTimeSecs = performance_timer.durationInSeconds();

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



    switch (message) {

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

        case WM_SYSKEYDOWN:

        case WM_KEYDOWN:
            Input::getInstance()->updateLastKeyPress(wParam);
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


