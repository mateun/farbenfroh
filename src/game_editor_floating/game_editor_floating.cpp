//
// Created by mgrus on 13.04.2025.
//



// TerminalShell_GDI.cpp - Pure Win32 Terminal Shell using GDI

// #define UNICODE
// #define _UNICODE
#include <Windows.h>
#include <string>
#include <vector>
#include <gdiplus.h>

extern void createNewGameDialog(HINSTANCE hInstance, HWND parentWindow);
void showConsoleWindow();

static LPCSTR gClassName = "GameEditorFloating";
static LPCSTR gWindowTitle = "GameEditor v0.0.1";


#define PRIMARY_TEXT_COLOR RGB(255, 255, 85)
#define SYNTAX_ERROR_COLOR RGB(255, 0, 0)
#define PRIMARY_BG_COLOR RGB(0, 0, 170)

HFONT gFont;
static SOCKET g_clientSocket = INVALID_SOCKET;
static int g_win_height = 600;
static int g_win_width = 800;
static int g_main_xPos = 0;
static int g_main_yPos = 0;
static std::string g_type_prop = "";    // The current type-ahead proposition
static std::vector<std::string> gLines = { "Game Engine Shell, version 0.0.1" };        // The raw lines which are printed on WM_PAINT (include the prompt sign etc.)
static std::vector<std::string> gInputs = {};   // The raw inputs of the user - can be used for history
static std::string gInput;
static int historyIndex = 0;
static bool g_cursor_visible = true;

// Editor Windows
HWND g_mainHwnd;
HWND g_helpHwnd;
HWND g_consoleHwnd;

HINSTANCE g_hinstance;
ULONG_PTR g_GdiPlusToken;

HBITMAP g_hScanlineBitmap = NULL;

// Command IDS

static const uint32_t ID_MENU_NEW_GAME = 100;
static const uint32_t ID_MENU_NEW_GAME_OBJECT = 101;
static const uint32_t ID_MENU_NEW_LEVEL = 102;
static const uint32_t ID_MENU_WINDOW_CONSOLE = 103;

void createMainMenu(HWND hwnd) {
    auto mainMenu = CreateMenu();
    auto menuFile = CreatePopupMenu();
    auto menuFileNew = CreatePopupMenu();
    AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)menuFile, "&File");
    AppendMenu(menuFile, MF_POPUP, (UINT_PTR) menuFileNew, "&New");
    AppendMenu(menuFileNew, MF_STRING, ID_MENU_NEW_GAME, "&Game");
    AppendMenu(menuFileNew, MF_STRING, ID_MENU_NEW_LEVEL, "&Level");
    AppendMenu(menuFileNew, MF_STRING, ID_MENU_NEW_GAME_OBJECT, "&Game Object");

    auto windowMenu = CreatePopupMenu();
    AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)windowMenu, "&Window");
    AppendMenu(windowMenu, MF_STRING, ID_MENU_WINDOW_CONSOLE, "Console");

    SetMenu(hwnd, mainMenu);
}

void CreateScanlineOverlay(int width, int height)
{
    HDC hdcScreen = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdcScreen);

    // Create a bitmap to hold the scanline overlay
    g_hScanlineBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, g_hScanlineBitmap);

    // Fill with a transparent background (if needed) or just leave as is
    Gdiplus::Graphics graphics(memDC);
    Gdiplus::Rect rect(0, 0, width, height);
    // Optionally clear with transparent color (depends on your composition)
    Gdiplus::SolidBrush transparentBrush(Gdiplus::Color(255, 0, 0, 145));
    graphics.FillRectangle(&transparentBrush, rect);

    // Draw scanlines once into the bitmap
    Gdiplus::SolidBrush scanlineBrush(Gdiplus::Color(128, 0, 0, 0)); // Semi-transparent black
    for (int y = 0; y < height; y += 2)
    {
        graphics.FillRectangle(&scanlineBrush, 0, y, width, 1);
    }

    // Cleanup
    SelectObject(memDC, oldBitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, hdcScreen);
}

void updateActualInput(const std::string& newInput) {
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


LRESULT CALLBACK ConsoleWndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    switch (msg)
    {

        case WM_KEYUP:
            {
                UINT vkcode = (UINT) w;
                if (VK_TAB == vkcode) {
                    //processSpecialKey(vkcode);
                }
                else if (VK_UP == vkcode) {
                    historyIndex++;
                    int actualIndex = gInputs.size() - historyIndex;
                    if (actualIndex >= 0) {
                        gInput = gInputs[actualIndex];
                        InvalidateRect(hwnd, nullptr, FALSE);
                    } else {
                        historyIndex--;
                    }
                }
                else if (VK_DOWN == vkcode) {
                    historyIndex--;
                    int actualIndex = gInputs.size() - historyIndex;
                    if (actualIndex < gInputs.size()) {
                        gInput = gInputs[actualIndex];
                        InvalidateRect(hwnd, nullptr, FALSE);
                    } else {
                        historyIndex++;
                    }


                }
                break;

            }

        case WM_CHAR:
            // if (w == VK_TAB) processSpecialKey((UINT)w);
            // else if (w == VK_BACK) { if (!gInput.empty()) gInput.pop_back(); }
            // else if (w == VK_RETURN) {
            //
            //     gLines.push_back("> " + gInput);
            //     //gLines.push_back("Executed: " + gInput);
            //     // Processing
            //     {
            //         // Validate and tokenize the input:
            //         auto tokens = tokenize(gInput);
            //         auto parseResultError = parse(tokens);
            //         if (parseResultError) {
            //             gLines.push_back("syntax error!");
            //         } else {
            //             gLines.push_back("cmd ok!");
            //         }
            //         //tcp_client_send(g_clientSocket, gInput);
            //     }
            //     gInputs.push_back(gInput);
            //     historyIndex = 0;
            //     gInput.clear();
            // }
            // else {
            //     gInput.push_back(w);
            //     processKeystroke(w);
            // }

            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;


        case WM_MOVE: {
            RECT windowRect;
            if(GetWindowRect(g_mainHwnd, &windowRect))
            {
                // The window's position is given by the left and top of the RECT.
                int x = windowRect.left;
                int y = windowRect.top;

                // Calculate width and height.
                int width = windowRect.right - windowRect.left;
                int height = windowRect.bottom - windowRect.top;

                SetWindowPos(g_helpHwnd, NULL, x + width + 10, y, 400, 200, 0);

            }
        }

        case WM_TIMER:
            g_cursor_visible = !g_cursor_visible;
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_DESTROY:
            DeleteObject(gFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, w, l);
}




LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    switch (msg)
    {

        case WM_COMMAND: {

            switch (w) {
                case ID_MENU_WINDOW_CONSOLE: showConsoleWindow();break;
                case ID_MENU_NEW_GAME: createNewGameDialog(g_hinstance, g_mainHwnd);break;
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
            if(GetWindowRect(g_mainHwnd, &windowRect))
            {
                // The window's position is given by the left and top of the RECT.
                int x = windowRect.left;
                int y = windowRect.top;

                // Calculate width and height.
                int width = windowRect.right - windowRect.left;
                int height = windowRect.bottom - windowRect.top;

                SetWindowPos(g_helpHwnd, NULL, x + width + 10, y, 400, 200, 0);

            }
            break;
        }

        // case WM_TIMER:
        //     g_cursor_visible = !g_cursor_visible;
        //     InvalidateRect(hwnd, nullptr, FALSE);
        //     return 0;

        case WM_DESTROY:
            DeleteObject(gFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, w, l);
}


void openHelpWindow() {

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = HelpWindowProc;
    wc.hInstance = g_hinstance;
    wc.lpszClassName = "HelpWindowClass";
    wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(155, 0, 0)));
    RegisterClass(&wc);

    RECT windowRect;
    if(GetWindowRect(g_mainHwnd, &windowRect))
    {
        // The window's position is given by the left and top of the RECT.
        int x = windowRect.left;
        int y = windowRect.top;

        // Calculate width and height.
        int width = windowRect.right - windowRect.left;
        int height = windowRect.bottom - windowRect.top;

        g_helpHwnd = CreateWindowEx(0, "HelpWindowClass", "Help", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        x + width + 5 , y, 400, 200, g_mainHwnd, nullptr, g_hinstance, nullptr);

    }


}

void showConsoleWindow() {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = ConsoleWndProc;
    wc.hInstance = g_hinstance;
    wc.lpszClassName = "ConsoleWindowClass";
    wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(2, 0, 0)));
    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    g_main_xPos = (screenWidth - g_win_width) / 2;
    g_main_yPos = (screenHeight - g_win_height - 100) ;

    g_mainHwnd = CreateWindowEx(0, "ConsoleWindowClass", "GameShell Console", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        g_main_xPos + g_win_width + 5, g_main_yPos, g_win_width, g_win_height, g_mainHwnd, nullptr, g_hinstance, nullptr);

    CreateScanlineOverlay(g_win_width, g_win_height);
    SetTimer(g_mainHwnd, 1, 500, NULL); // 500ms blink
}



int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int)
{


    // Initialize GDI+.
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_GdiPlusToken, &gdiplusStartupInput, NULL);

    g_hinstance = h;
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = h;
    wc.lpszClassName = gClassName;
    wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(2, 0, 0)));
    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    g_main_xPos = (screenWidth - g_win_width) / 2;
    g_main_yPos = (screenHeight - g_win_height - 100) ;

    g_mainHwnd = CreateWindowEx(0, gClassName, gWindowTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        g_main_xPos, g_main_yPos, g_win_width, g_win_height, nullptr, nullptr, h, nullptr);

    gFont = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Courier");


    createMainMenu(g_mainHwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Gdiplus::GdiplusShutdown(g_GdiPlusToken);
    return 0;
}

