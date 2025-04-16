//
// Created by mgrus on 13.04.2025.
//



// TerminalShell_GDI.cpp - Pure Win32 Terminal Shell using GDI

// #define UNICODE
// #define _UNICODE

#define UNICODE
#include <algorithm>
#include <Windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <gdiplus.h>
#include <windowsx.h>
#include <engine/graphics/Widget.h>

#include "editor_data.h"
#include "paint_2d.h"
#include "fonts.h"
#include "d2d_ui.h"
#include <game_editor_floating/standard_widgets/standard_widgets.h>


#define PRIMARY_TEXT_COLOR RGB(255, 255, 85)
#define SYNTAX_ERROR_COLOR RGB(255, 0, 0)
#define PRIMARY_BG_COLOR RGB(0, 0, 170)

extern HWND createSplahIntroWindow(HWND parentWindow, HINSTANCE hInstance);
extern HWND createGameObjectTreeWindow(HWND hwnd, HINSTANCE hInstance);
extern void createNewGameDialog(HINSTANCE hInstance, HWND parentWindow);
extern void createEmptyLevel(const std::string& name, const std::string& projectFolder, HWND parentWindow);

static LRESULT CALLBACK GameObjectTreeProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l);
static std::wstring gClassName = L"GameEditorFloating";
static std::wstring gWindowTitle = L"GameEditor v0.0.1";

static IDWriteTextFormat* orbitron_bold;
static IDWriteTextFormat* orbitron_small;


HFONT gFont;
static SOCKET g_clientSocket = INVALID_SOCKET;
static HBITMAP g_hScanlineBitmap = nullptr;
static int g_win_height = 600;
static int g_win_width = 800;
static int g_main_xPos = 0;
static int g_main_yPos = 0;
static std::string g_type_prop = "";    // The current type-ahead proposition
static std::vector<std::string> gLines = { "Game Engine Shell, version 0.0.1" };        // The raw lines which are printed on WM_PAINT (include the prompt sign etc.)
static std::vector<std::string> gInputs = {};   // The raw inputs of the user - can be used for history
static std::string gInput;
static int historyIndex = 0;
static bool resizing_ = false;
static int header_height_ = 40;
static bool g_cursor_visible = true;

// Editor Windows
HWND g_mainHwnd;
HWND g_helpHwnd;
HWND g_consoleHwnd;
static HWND g_objectTreeHwnd;
static HWND g_splashIntroWindow;

static HINSTANCE g_hinstance;
static ULONG_PTR g_GdiPlusToken;

// UI framework stuff:
d2d_Widget* hoveredWidget = nullptr;
d2d_Widget* lastHoveredWidget = nullptr;
d2d_Widget* capturing_widget = nullptr;
HCURSOR current_cursor = LoadCursor(NULL, IDC_ARROW);
HCURSOR crs_arrow  = current_cursor;
HCURSOR crs_size_nwse = LoadCursor(nullptr, IDC_SIZENWSE);
static std::vector<d2d_Widget*> widgets_;

// My editor widgets:
d2d_Widget * headerBGWidget = nullptr;
d2d_Widget * closeButton = nullptr;

// Command IDS
static const uint32_t ID_MENU_NEW_GAME = 100;
static const uint32_t ID_MENU_NEW_GAME_OBJECT = 101;
static const uint32_t ID_MENU_NEW_LEVEL = 102;
static const uint32_t ID_MENU_WINDOW_CONSOLE = 103;

/**
 * Creates the editors (application) main toplevel menu.
 * @param hwnd The main window handle
 */
void createMainMenu(HWND hwnd) {
    auto mainMenu = CreateMenu();
    auto menuFile = CreatePopupMenu();
    auto menuFileNew = CreatePopupMenu();
    AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)menuFile, L"&File");
    AppendMenu(menuFile, MF_POPUP, (UINT_PTR) menuFileNew, L"&New");
    AppendMenu(menuFileNew, MF_STRING, ID_MENU_NEW_GAME, L"&Game");
    AppendMenu(menuFileNew, MF_STRING, ID_MENU_NEW_LEVEL, L"&Level");
    AppendMenu(menuFileNew, MF_STRING, ID_MENU_NEW_GAME_OBJECT, L"&Game Object");

    auto windowMenu = CreatePopupMenu();
    AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)windowMenu, L"&Window");
    AppendMenu(windowMenu, MF_STRING, ID_MENU_WINDOW_CONSOLE, L"Console");

    SetMenu(hwnd, mainMenu);
}

static void updateActualInput(const std::string& newInput) {
    size_t lastSpacePos = gInput.find_last_of(' ');
    if (lastSpacePos == std::string::npos) {
        gInput = newInput;
    } else {
        auto shortened = gInput.substr(0, lastSpacePos+1);
        gInput = shortened+ newInput;
    }

}

void updateTypeProp(const std::string& newTypeProp) {
    g_type_prop = newTypeProp;
}

std::string getTypeProp() {
    return g_type_prop;
}

void clearTypeProp() {
    g_type_prop.clear();
}


LRESULT CALLBACK HelpWindowProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    return DefWindowProc(hwnd, msg, w, l);
}


// This gets called on WM_SIZE and we must resize all size-critical widgets here, e.g. header, close box etc.
void resize_editor_widgets() {
    if (headerBGWidget) {
        headerBGWidget->bounds_.right = g_win_width;
    }
    if (closeButton) {
        closeButton->bounds_ = { g_win_width - 36.0f, 8, (float)g_win_width-8.0f, (float) header_height_ };
    }


}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    switch (msg)
    {

        case WM_COMMAND: {
            switch (w) {
                // case ID_MENU_WINDOW_CONSOLE: showConsoleWindow();break;
                case ID_MENU_NEW_GAME: createNewGameDialog(g_hinstance, g_mainHwnd);break;
            }
            break;
        }

        case WM_PAINT:
        {
            // Begin/EndPaint are important, otherwise flickering!
            // If we catch WM_PAINT, it must be wrapped inside this bracket!
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);

            ////////////////////////////////////////////////////////////////////////////////////////
            // Rendering
            // Use our widget "framework"
            // Here we just naively implement the framework render function.
            // We may want to move this into a separate convencience struct/function, but for now
            // this is how a client application would use the framework:
            // Begin d2d draw.
            // Clear everything with d2d.
            // Sort the widgets by z-order ascending.
            // Iterate all the widgets and call draw on each.
            // End d2d draw.
            paint2d_begin_draw();
            paint2d_clear({0x0a, 0x0a, 0x0a, 0xff});
            std::ranges::sort(widgets_, [](d2d_Widget* w1, d2d_Widget* w2) {
                return w1->z < w2->z;
            });
            for (auto& w : widgets_) {
                w->draw();
            }
            paint2d_end_draw();
            ////////////////////////////////////////////////////////////////////////////////////////

            EndPaint(hwnd, &ps);

            return 0;
        }

        // We only allow resizing on bottom left corner:
        case WM_NCHITTEST:
            {
            POINT pt = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
            ScreenToClient(hwnd, &pt);
            RECT rc;
            GetClientRect(hwnd, &rc);

            const int border = 16;

            if (pt.y >= rc.bottom - border && pt.x >= rc.right - border) {
                resizing_ = true;
                return HTBOTTOMRIGHT;
            }

            resizing_ = false;
            return HTCLIENT;
        }

        case WM_MOUSEMOVE: {
            lastHoveredWidget = hoveredWidget;
            hoveredWidget = nullptr;

            POINT pt = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };

            auto reverse_sorted = widgets_;
            std::ranges::sort(reverse_sorted, [](d2d_Widget* w1, d2d_Widget* w2) {
                return w1->z > w2->z;
            });
            for (auto it = widgets_.rbegin(); it != widgets_.rend(); ++it)
            {
                if ((*it)->hitTest(pt)) {
                    printf("hoverwidget found!\n");
                    hoveredWidget = *it;
                    break;
                }
            }

            if (capturing_widget) {
                capturing_widget->on_mouse_move(pt.x, pt.y);
            }
            else if (hoveredWidget) {
                hoveredWidget->on_mouse_move(pt.x, pt.y);
                HCURSOR desired_cursor = hoveredWidget->get_cursor();
                if (desired_cursor != current_cursor) {
                    current_cursor = desired_cursor;
                }
            } else {
                current_cursor = crs_arrow;
            }

            if (hoveredWidget != lastHoveredWidget) {
                static int counter = 0;
                counter++;
                if (lastHoveredWidget) {
                    printf("set hover false (%d)!\n", counter);
                    lastHoveredWidget->isHovered_ = false;
                }
                if (hoveredWidget) {
                    printf("set hover true (%d)!\n", counter);
                    hoveredWidget->isHovered_ = true;
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            } else {
                printf("no change in widget hovering!\n");
            }

            break;
        }
        case WM_ERASEBKGND:
            return 1;  // Don't clear → we fully redraw in WM_PAINT


        case WM_SIZE: {
            g_win_width = LOWORD(l);
            g_win_height = HIWORD(l);
            paint2d_resize(g_win_width, g_win_height);
            resize_editor_widgets();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            POINT pt = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
            // if (pt.y < header_height_) { // or a real hit test
            //     ReleaseCapture();
            //     SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            // }

            if (hoveredWidget) {
                hoveredWidget->on_mouse_down(pt.x, pt.y);
                if (hoveredWidget->wants_capture_on_mouse_down()) {
                    SetCapture(hwnd);
                    capturing_widget = hoveredWidget;
                }
                return 0;
            }
            break;
        }

        case WM_LBUTTONUP: {
            if (hoveredWidget) {
                hoveredWidget->onClick();
            };

            if (capturing_widget) {
                POINT pt = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
                capturing_widget->on_mouse_up(pt.x, pt.y);
                capturing_widget = nullptr;
                ReleaseCapture();
            }
            break;
        }

        case WM_SETCURSOR: {
            if (resizing_) {
                current_cursor = crs_size_nwse;
            }

            SetCursor(current_cursor);

            return TRUE;
        }
        case WM_DESTROY: {
            DeleteObject(gFont);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, w, l);
}



/**
 * Called when the user created a new project.
 * @param data Mainly name of the project and folder location
 */
void onNewGameCreated(const NewGameData& data) {
    createEmptyLevel("Level1", data.projectLocation + "/Assets/Levels/main.json", g_mainHwnd);
    // loadLevel(data.projectLocation + L"/Assets/Levels/main.json");
    ShowWindow(g_objectTreeHwnd, SW_HIDE);
}

struct d2d_HeaderWidget : d2d_Widget {
    void draw() override {
        if (isHovered_) {
            paint2d_draw_filled_rect({0, 0}, {g_win_width, 48}, {0x1a, 0x1a, 0x1a, 255});
        }
    }
};

d2d_Widget* createTopTitleLabel(std::wstring title) {
    auto w = new d2d_Label(title);
    w->bounds_ = {16, 8, 300, 36};
    w->font_name_ = L"../assets/Orbitron-VariableFont_wght.ttf";
    w->text_color_ = {200, 200, 200, 255};
    w->font_size_ = 20.0f;
    w->z = 20;
    return w;

}

d2d_Widget* createHeaderBg() {
    auto w = new d2d_GripWidget(g_mainHwnd);
    w->bounds_ = { 0, 0, (float)g_win_width - 0, (float) header_height_ };
    w->orientation_ = GripOrientation::Horizontal;
    w->dotRadius = 1.2f;
    w->debugDrawBounds_ = false;
    w->spacing = 6.0f;
    w->dotColor = {85, 85, 85, 255};
    w->z = -3;
    return w;
}

d2d_Widget* createToolbar() {
    return {};
}

d2d_Widget* createCloseButtonWidget() {
    auto cbWidget = new d2d_CloseButtonWidget();
    cbWidget->bounds_ = { g_win_width - 36.0f, 8, (float)g_win_width-8.0f, (float) header_height_ };
    cbWidget->z = 100;  // really on top
    return cbWidget;
}

#ifdef console__
void showConsoleWindow() {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = g_hinstance;
    wc.lpszClassName = L"ConsoleWindowClass";
    wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(2, 0, 0)));
    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    g_main_xPos = (screenWidth - g_win_width) / 2;
    g_main_yPos = (screenHeight - g_win_height - 100) ;

    auto consoleWindow = CreateWindowEx(0, L"ConsoleWindowClass", L"GameShell Console", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        g_main_xPos + g_win_width + 5, g_main_yPos, g_win_width, g_win_height, g_mainHwnd, nullptr, g_hinstance, nullptr);

    SetTimer(consoleWindow, 1, 500, NULL); // 500ms blink
}
#endif

/*
    Palette
    Purpose | Hex Color | Notes
    Background | #0A0A0A | Almost black
    Panel/Surface | #1A1A1A | Very dark gray
    UI Lines | #3A3A3A | Mid-gray separation lines
    Accent Red | #E03C31 |  style blood orange
    Toxic Green | #3BEA55 | Status lights, highlights
    Warning Yellow | #EFBF1A | Optional caution color
    Soft White Text | #EAEAEA | Comfortable on dark bg
    Hard White Text | #FFFFFF | For small UI elements
 */




int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int)
{
    // Initialize GDI+.
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_GdiPlusToken, &gdiplusStartupInput, NULL);




    g_hinstance = h;
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = h;
    wc.lpszClassName = gClassName.c_str();
    wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(0x0A, 0x0A, 0x0A)));   // Actually not needed as we redraw with d2d on every WM_PAINT
    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    g_main_xPos = (screenWidth - g_win_width) / 2;
    g_main_yPos = (screenHeight - g_win_height - 100) ;

    g_mainHwnd = CreateWindowEx(0, gClassName.c_str(), gWindowTitle.c_str(), WS_POPUP | WS_VISIBLE,
        g_main_xPos, g_main_yPos, g_win_width, g_win_height, nullptr, nullptr, h, nullptr);

    gFont = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Courier");

    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icex);

    //createMainMenu(g_mainHwnd);
    //g_objectTreeHwnd = createGameObjectTreeWindow(g_mainHwnd, g_hinstance);
    //ShowWindow(g_objectTreeHwnd, SW_HIDE);

    //g_splashIntroWindow = createSplahIntroWindow(g_mainHwnd, g_hinstance);

    paint2d_init(g_mainHwnd);
    createDWriteFont(L"../assets/Orbitron-VariableFont_wght.ttf", &orbitron_bold, 18.0f);
    createDWriteFont(L"../assets/Orbitron-VariableFont_wght.ttf", &orbitron_small, 12.0f);

    //////////////////////////////////////////////////////////////////////////////////////
    // Create widgets we need in our main editor window:
    auto orbi_font = L"../assets/Orbitron-VariableFont_wght.ttf";
    auto calibri_font = L"../assets/calibri.ttf";
    closeButton = createCloseButtonWidget();
    headerBGWidget = createHeaderBg();
    auto topTitleLabel = createTopTitleLabel(L"_BORST EDITOR");
    auto btnNewGame = new d2d_Button({16, 48, 16 + 9*8, 48 + 4*8});
    btnNewGame->label_ = new d2d_Label(L"New Game");
    btnNewGame->label_->font_name_ = calibri_font;
    btnNewGame->label_->font_size_ = 10;
    btnNewGame->label_->text_color_ = {0x0b, 0x0c, 0x0c, 255};

    btnNewGame->click_callback_ = []() {
        printf("new game clicked!\n");
    };

    widgets_.push_back(headerBGWidget);
    widgets_.push_back(closeButton);
    widgets_.push_back(topTitleLabel);
    widgets_.push_back(btnNewGame);
    //////////////////////////////////////////////////////////////////////////////////////


    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Gdiplus::GdiplusShutdown(g_GdiPlusToken);
    delete(headerBGWidget);
    return 0;
}

