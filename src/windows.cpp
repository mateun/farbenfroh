// TerminalShell_GDI.cpp - Pure Win32 Terminal Shell using GDI

#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <string>
#include <vector>

LPCWSTR gClassName = L"TerminalShellGDI";
LPCWSTR gWindowTitle = L"Game Engine Shell";

std::vector<std::wstring> gLines = { L"Welcome to Game Engine Shell  dfdf!" };
std::wstring gInput;

HFONT gFont;

void Render(HDC hdc)
{
    RECT client;
    GetClientRect(WindowFromDC(hdc), &client);

//    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
//    SelectObject(hdc, blackBrush); //select brush into DC
//    FillRect(hdc, &client, blackBrush);
//    DeleteObject(blackBrush);

    //SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 191, 0)); // Amber

    int lineHeight = 20;
    int y = 10;

    for (const auto& line : gLines)
    {
        TextOutW(hdc, 10, y, line.c_str(), (int)line.size());
        y += lineHeight;
    }

    std::wstring prompt = L"> " + gInput + L"_";
    TextOutW(hdc, 10, y, prompt.c_str(), (int)prompt.size());
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
    switch (msg)
    {
    case WM_CHAR:
        if (w == VK_BACK) { if (!gInput.empty()) gInput.pop_back(); }
        else if (w == VK_RETURN) {
            gLines.push_back(L"> " + gInput);
            gLines.push_back(L"Executed: " + gInput);
            gInput.clear();
        }
        else gInput.push_back((wchar_t)w);

        //InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        //SelectObject(hdc, gFont);
        //Render(hdc);
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
//    WNDCLASS wc = { 0 };
//    wc.lpfnWndProc = WndProc;
//    wc.hInstance = h;
//    wc.lpszClassName = gClassName;
//    wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(0, 0, 0)));
//    RegisterClass(&wc);
//
//    HWND hwnd = CreateWindowEx(0, gClassName, gWindowTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
//        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, h, nullptr);
//
//    gFont = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
//        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
//        FIXED_PITCH | FF_MODERN, L"Consolas");
//
//    MSG msg = {};
//    while (GetMessage(&msg, nullptr, 0, 0))
//    {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }

    return 0;
}
