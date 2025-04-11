//
// Created by mgrus on 23.03.2025.
//

#include "Application.h"


#include <commctrl.h>
#include <ranges>



#include <engine/profiling/PerformanceTimer.h>
#include <engine/graphics/MeshDrawData.h>
#include <GL/glew.h>
#include "RenderBackend.h"
#include <engine/graphics/Widget.h>
#include <engine/game/Timing.h>
#include <engine/input/Input.h>


#include <engine/graphics/RawWin32Message.h>
#include <engine/graphics/ui/FrameMessageSubscriber.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/ui/FocusManager.h>
#include <engine/graphics/ui/MessageDispatcher.h>

#include <filesystem>
#include <engine/io/Logger.h>



#include "TrueTypeFont.h"
#include "ui/CentralSubMenuManager.h"
#include "ui/FloatingWindow.h"
#include "ui/MenuBar.h"

#include <gdiplus.h>



// This function must be provided by any Application implementor.
extern std::shared_ptr<Application> getApplication();

Application::Application(int w, int h, bool fullscreen) : width_(w), height_(h), fullscreen(fullscreen), scaled_width_(width_), scaled_height_(height_) {
    logger_ = std::make_shared<Logger>(log_stream_);
}

Application::~Application() {
    this->frame_messages_.clear();
    this->messageSubscribers.clear();

}


bool Application::changeResolution(int width, int height, int refreshRate, const std::string& deviceName, bool goFullscreen) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    width_ = width;
    height_ = height;

    auto dpi = GetDpiForWindow(window_);
    auto dpiScaleFactor = static_cast<float>(dpi) / 96.0f;
    scaled_width_ = width / dpiScaleFactor;
    scaled_height_ = height / dpiScaleFactor;

    if (topLevelWidget) {
        topLevelWidget->setSize({scaled_width_, scaled_height_});
    }

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
        SetWindowLong(window_, GWL_STYLE, WS_POPUP);
        SetWindowPos(window_, HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED);
        ShowWindow(window_, SW_MAXIMIZE);

        // Update our internal window size variables:

        fullscreen = true;

    }


    return true;

}

std::vector<RawWin32Message> Application::getLastMessages() {
    return frame_messages_;
}

void Application::setMainMenuBar(const std::shared_ptr<MenuBar> &mainMenuBar) {
    main_menu_bar_ = mainMenuBar;
    getApplication()->getCentralSubMenuManager()->registerMenuBar(mainMenuBar);
}

HBITMAP Application::loadBitmapFromFile(const std::wstring& fileName) {
    Gdiplus::Bitmap bitmap(fileName.c_str());
    HBITMAP hBitmap;
    bitmap.GetHBITMAP(Gdiplus::Color::Transparent, &hBitmap);
    return hBitmap;

}

void Application::initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpstr, int nShowCmd) {

    h_instance_ = hInstance;
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

    RECT corrRect = {0, 0, width_, height_};
    AdjustWindowRect(&corrRect, WS_OVERLAPPEDWINDOW, false);

    auto winWidthHalf = (corrRect.right  - corrRect.left) / 2;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);


    window_ = CreateWindow(
			 g_szClassName,
             "Application",
			 //WS_POPUP,
			 WS_OVERLAPPEDWINDOW,
			 screenWidth/2 - winWidthHalf, 0, corrRect.right - corrRect.left, corrRect.bottom - corrRect.top,
			 NULL, NULL, hInstance, this);



    if (window_ == NULL) {
        MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    }

    ShowWindow(window_, SW_NORMAL);
    UpdateWindow(window_);
    hdc = GetDC(window_);
    render_backend_ = std::make_unique<RenderBackend>(RenderBackendType::OpenGL, hdc, window_, width_, height_);

    // Load the resize cursor (horizontal resize cursor)
    resize_cursor_horizontal_ = LoadCursor(NULL, IDC_SIZEWE);
    resize_cursor_vertical_ = LoadCursor(NULL, IDC_SIZENS);
    text_edit_cursor = LoadCursor(NULL, IDC_IBEAM);
    hand_cursor_ = LoadCursor(NULL, IDC_HAND);

    central_submenu_manager_ = std::make_shared<CentralSubMenuManager>();
    focus_manager_ = std::make_shared<FocusManager>();
    focus_based_message_dispatcher_ = std::make_shared<FocusBasedMessageDispatcher>(*focus_manager_);
    simple_message_dispatcher_ = std::make_shared<SimpleMessageDispatcher>();

    addMessageSubscriber(simple_message_dispatcher_);
    addMessageSubscriber(focus_manager_);
    addMessageSubscriber(focus_based_message_dispatcher_);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

}

int Application::run() {
    onCreated();
	mainLoop();
	return 0;
}

RenderBackend * Application::getRenderBackend() const {
    return render_backend_.get();
}

void Application::addMessageSubscriber(std::shared_ptr<FrameMessageSubscriber> subscriber) {
    messageSubscribers.push_back(subscriber);
}

int Application::scaled_width() {
    return scaled_width_;
}

int Application::scaled_height() {
    return scaled_height_;
}

std::shared_ptr<Widget> Application::getTopLevelWidget() {
    return topLevelWidget;
}

#ifdef USE_WIN32_APP_FRAMEWORK
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nShowCmd) {

    // This should catch heap corruption - but does not, so maybe I don't have any anyway.
    // But it slows down the program completely, so only turn it off when really needed:
#ifdef DEBUG_HEAP_CORRUPTION
    // int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    // dbgFlags |= _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF;
    // _CrtSetDbgFlag(dbgFlags);
#endif

	std::shared_ptr<Application> app = getApplication();
	app->initialize(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
	return app->run();

}
#endif


void Application::setTopLevelWidget(const std::shared_ptr<Widget>& widget) {
    topLevelWidget = widget;

    // The top level widget is always the full application window size.
    // Further down the hierarchy, e.g. splitted windows have smaller sizes.
    topLevelWidget->setSize({width_, height_});
    topLevelWidget->setZValue(std::numeric_limits<float>::lowest());
}

void Application::addFloatingWindow(std::shared_ptr<FloatingWindow> window) {
    floating_windows_.push_back(window);
}

void Application::removeFloatingWindow(std::shared_ptr<FloatingWindow> floating_window) {
    floating_windows_closed_.push_back(floating_window);
}

std::shared_ptr<TrueTypeFont> Application::getDefaultMenuFont() {
    if (!font_) {
        // FIXME fontsize other than 13 currently breaks the menu
        font_ = std::make_shared<TrueTypeFont>("../assets/calibri.ttf", 13);
    }

    return font_;

}

int Application::width() {
    return width_;
}

int Application::height() {
    return height_;
}

std::shared_ptr<Widget> Application::getMenuBar() {
    return main_menu_bar_;
}

void Application::createNativeToolbar() {
    // Declare and initialize local constants.
    const int ImageListID    = 0;
    const int numButtons     = 3;
    const int bitmapSize     = 16;

    const DWORD buttonStyles = BTNS_AUTOSIZE;

    // Create the toolbar.
    hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                                      WS_CHILD | TBSTYLE_WRAPABLE, 0, 0, 0, 0,
                                      window_, NULL, h_instance_, NULL);

    if (hWndToolbar == NULL)
        throw std::runtime_error("CreateWindowEx for toolbar failed");

    // Create the image list.
    main_toolbar_image_list_ = ImageList_Create(bitmapSize, bitmapSize,   // Dimensions of individual bitmaps.
                                    ILC_COLOR32,
                                    numButtons, 0);

    // Set the image list.
    SendMessage(hWndToolbar, TB_SETIMAGELIST,
                (WPARAM)ImageListID,
                (LPARAM)main_toolbar_image_list_);

    // Load the button images.
    SendMessage(hWndToolbar, TB_LOADIMAGES,
                (WPARAM)IDB_STD_SMALL_COLOR,
                (LPARAM)HINST_COMMCTRL);

    // Initialize button info.
    // IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.

    TBBUTTON tbButtons[numButtons] =
    {
        { MAKELONG(STD_FILENEW,  ImageListID), IDM_NEW,  TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"New" },
        { MAKELONG(STD_FILEOPEN, ImageListID), IDM_OPEN, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"Open"},
        { MAKELONG(STD_FILESAVE, ImageListID), IDM_SAVE, 0,               buttonStyles, {0}, 0, (INT_PTR)L"Save"}
    };

    // Add buttons.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS,       (WPARAM)numButtons,       (LPARAM)&tbButtons);

    // Resize the toolbar, and then show it.
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
    ShowWindow(hWndToolbar,  TRUE);

}

void Application::addMainToolbarButton(const std::wstring &bitmapFileName) {
    auto hbm = loadBitmapFromFile(bitmapFileName);


    int imageIndex = ImageList_Add(main_toolbar_image_list_, hbm, NULL);
    TBBUTTON tbButton = {};
    tbButton.iBitmap = imageIndex; // this is the index in the image list.
    tbButton.idCommand = 123; // Your command identifier.
    tbButton.fsState   = TBSTATE_ENABLED;
    tbButton.fsStyle   = BTNS_BUTTON;
    tbButton.iString   = (INT_PTR)L"Custom";

    // Add the button to the toolbar as usual.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbButton);
}


HWND Application::hwnd() {
    return window_;
}

void Application::doFrame() {
    // noop
}

void Application::clearClosedFloatingWindows() {
    for (auto cfw : floating_windows_closed_) {
        std::erase(floating_windows_, cfw);
    }
}

void Application::setAllowCursorOverride(bool allow) {
    allow_cursor_override_ = allow;
}

bool Application::allowCursorOverride() {
    return allow_cursor_override_;
}

void Application::setSpecialCursor(CursorType cursorType) {

    switch (cursorType) {
        case CursorType::ResizeHorizontal: SetCursor(resize_cursor_horizontal_); break;
        case CursorType::ResizeVertical: SetCursor(resize_cursor_vertical_); break;
        case CursorType::TextEdit: SetCursor(text_edit_cursor);break;
        case CursorType::Hand: SetCursor(hand_cursor_); break;
    }
    allow_cursor_override_ = true;

    // Restore the previous cursor when done?!
    //SetCursor(hOldCursor);
}

void Application::unsetSpecialCursor() {
    allow_cursor_override_ = false;
}

std::shared_ptr<CentralSubMenuManager> Application::getCentralSubMenuManager() {
    return central_submenu_manager_;
}

std::shared_ptr<FocusManager> Application::getFocusManager() {
    return focus_manager_;
}

std::vector<std::shared_ptr<FloatingWindow>> Application::getFloatingWindows() {
    return floating_windows_;
}

glm::vec2 Application::getCurrentMousePos() {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(window_, &pt);
    return glm::vec2(pt.x, scaled_height_ - pt.y);
}

void Application::mainLoop() {

    //initXInput();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	bool running = true;

    PerformanceTimer performance_timer;

    MSG msg;
	while (running) {
	    performance_timer.start();
	    frame_messages_.clear();
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                running = false;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

	    // Call our frame-based logic function.
	    doFrame();

	    clearClosedFloatingWindows();

	    // Send raw frame messages to all subscribers,
	    // unless we are currently shutting off the messages.
	    // This might be the case when we have a native modal dialog opened,
	    // which interferes with our regular message pump flow.
	    if (!temp_ignore_messages_) {
	        for (auto& msgSub : messageSubscribers) {
	            msgSub->onFrameMessages(frame_messages_);
	        }
	    }


	    if (topLevelWidget) {
	        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	        // We provide an ortho camera which represents the application window dimensions.
	        render_backend_->setViewport(0,0, scaled_width(), scaled_height());

	        if (main_menu_bar_) {
	            main_menu_bar_->setOrigin({0, height_ - 32});
	            main_menu_bar_->setSize({width_, 32});
	            main_menu_bar_->draw(-20);
	        }


	        topLevelWidget->draw(-20);
            Renderer::submitDeferredWidgetCalls();

            // Render all floating windows:
	        for (auto& fw : floating_windows_) {
	            fw->draw(1);
	        }
	        // TODO maybe render all draw calls at once?!
	        Renderer::submitDeferredWidgetCalls();

	        // // Finally present to the main framebuffer.
	         SwapBuffers(hdc);
	    }
	    performance_timer.stop();
	    Timing::frameTimeSecs = performance_timer.durationInSeconds();

	}

}

LRESULT Application::AppWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Application* appPtr = nullptr;
    if (message == WM_NCCREATE) {
        // On WM_NCCREATE, extract the 'this' pointer from the CREATESTRUCT
        LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        appPtr = reinterpret_cast<Application*>(pcs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(appPtr));
    } else {
        appPtr = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

#define ID_LEFT_CHILD  101
#define ID_RIGHT_CHILD 102
    if (appPtr) {
        appPtr->message_count++;
        appPtr->frame_messages_.push_back(RawWin32Message{message, wParam, lParam, appPtr->message_count});
    }

    switch (message) {

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDM_NEW: std::cout << "new clicked!!" << std::endl; break;
                default: break;
            }

            break;
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

        case WM_SIZE:
            appPtr->changeResolution(LOWORD(lParam), HIWORD(lParam), 120, "", false);
            break;

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
            // lParam's low-order word indicates the hit-test result.
            WORD hitTest = LOWORD(lParam);
            if (hitTest == HTCLIENT) {
                // In the client area, explicitly set the cursor to the standard arrow - unless the application
                // tells us to allow the cursor override:
                if (appPtr && appPtr->allowCursorOverride()) {
                    // do nothing, we just keep the current cursor set by the application
                } else {
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                }

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


