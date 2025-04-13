// TerminalShell_GDI.cpp - Pure Win32 Terminal Shell using GDI

// #define UNICODE
// #define _UNICODE
#include <Windows.h>
#include <string>
#include <vector>
#include "tcp_client.h"
#include "command_parser.h"

LPCSTR gClassName = "TerminalShellGDI";
LPCSTR gWindowTitle = "Game Engine Shell";

std::vector<std::string> gLines = { "Welcome to Game Engine Shell!" };
std::string gInput;

HFONT gFont;
SOCKET g_clientSocket = INVALID_SOCKET;
int g_win_height = 600;
int g_win_width = 800;

void Render(HDC hdc)
{
    RECT client;
    GetClientRect(WindowFromDC(hdc), &client);
    HBRUSH brush = CreateSolidBrush(RGB(5,0,0)); //create brush
    SelectObject(hdc, brush); //select brush into DC
    FillRect(hdc, &client, brush);
    DeleteObject(brush);

    SetBkMode(hdc, TRANSPARENT);


    int lineHeight = 20;
    int y = 10;

    int startLineIndex = 0;
    // How many lines can we display?
    int maxLinesToDisplay = (g_win_height / lineHeight) - 8;

    if (gLines.size() > maxLinesToDisplay) {
        startLineIndex =  (gLines.size() - maxLinesToDisplay);
    }

    for (int i = startLineIndex; i < gLines.size(); i++)
    {
        auto line = gLines[i];
        if (line.starts_with("syntax")) {
            SetTextColor(hdc, RGB(255, 10, 0)); // Amber
        } else {
            SetTextColor(hdc, RGB(255, 191, 0)); // Red
        }

        TextOut(hdc, 10, y, line.c_str(), (int)line.size());
        y += lineHeight;
    }

    // The prompt and the next line are always in normal color again:
    SetTextColor(hdc, RGB(255, 191, 0)); // Amber
    std::string prompt = "> " + gInput + "_";
    TextOut(hdc, 10, y, prompt.c_str(), (int)prompt.size());
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
    switch (msg)
    {
    case WM_CHAR:
        if (w == VK_BACK) { if (!gInput.empty()) gInput.pop_back(); }
        else if (w == VK_RETURN) {
            gLines.push_back("> " + gInput);
            //gLines.push_back("Executed: " + gInput);
            // Processing
            {
                // Validate and tokenize the input:
                auto tokens = tokenize(gInput);
                auto parseResultError = parse(tokens);
                if (parseResultError) {
                    gLines.push_back("syntax error!");
                } else {
                    gLines.push_back("cmd ok!");
                }
                //tcp_client_send(g_clientSocket, gInput);
            }

            gInput.clear();
        }
        else gInput.push_back(w);

        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SelectObject(hdc, gFont);
        Render(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        DeleteObject(gFont);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, w, l);
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int)
{

    if (tcp_client_start("localhost", "60781", g_clientSocket)) {
        // We ignore the tcp component for now, while testing! TODO
        //return 1;
    };

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = h;
    wc.lpszClassName = gClassName;
    wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(2, 0, 0)));
    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int xPos = (screenWidth - g_win_width) / 2;
    int yPos = (screenHeight - g_win_height - 100) ;

    HWND hwnd = CreateWindowEx(0, gClassName, gWindowTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        xPos, yPos, g_win_width, g_win_height, nullptr, nullptr, h, nullptr);

    gFont = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Courier");



    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
