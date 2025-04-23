//
// Created by mgrus on 15.04.2025.
//

#define UNICODE
#include "fonts.h"
#include <cstdio>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite_3.h>
#include <wincodec.h>

static ID2D1Factory* d2d_factory;
static ID2D1HwndRenderTarget* d2d_render_target;
static ID2D1SolidColorBrush* header_bg_brush;
static ID2D1SolidColorBrush* almost_black_brush;
static ID2D1SolidColorBrush* m_pTextHeaderBrush;
static ID2D1SolidColorBrush* m_pSeparatorLineCol;
static IDWriteFactory5* m_pDWriteFactory;
static IDWriteTextFormat* txtFormat_Normal;
static int splash_width;
static int splash_height;

static IDWriteTextFormat* orbitronNormal;


static void createDeviceIndependentResources() {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);

    if (SUCCEEDED(hr))
    {
        // Create a DirectWrite factory.
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory5),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }
    if (SUCCEEDED(hr))
    {

        // Setup our own local font(family):
        IDWriteFontFile* fontFile;
        m_pDWriteFactory->CreateFontFileReference(L"../assets/Orbitron-VariableFont_wght.ttf", nullptr, &fontFile);

        IDWriteFontSet* fontSet;
        IDWriteFontCollection1* fontCollection;
        IDWriteFontSetBuilder1* fontSetBuilder;
        m_pDWriteFactory->CreateFontSetBuilder(&fontSetBuilder);
        fontSetBuilder->AddFontFile(fontFile);
        fontSetBuilder->CreateFontSet(&fontSet);
        m_pDWriteFactory->CreateFontCollectionFromFontSet(fontSet, &fontCollection);

        // Create a DirectWrite text format object.
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Orbitron",
            fontCollection,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            14.0f,
            L"en-us", //locale
            &txtFormat_Normal
            );
    }
    if (SUCCEEDED(hr))
    {
        // Center the text horizontally and vertically.
        txtFormat_Normal->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        txtFormat_Normal->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    }
}

static void createDeviceResources(HWND hwnd) {

    HRESULT hr = S_OK;

    if (!d2d_render_target) {

        RECT rc;
        GetClientRect(hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top);

        // Create a Direct2D render target.
        auto hr = d2d_factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hwnd, size),
            &d2d_render_target);

        if (SUCCEEDED(hr))
        {
            // Create a gray brush.
            hr = d2d_render_target->CreateSolidColorBrush(
                D2D1::ColorF(0xC7FF00),
                &header_bg_brush
                );
        }

        if (SUCCEEDED(hr))
        {
            // Create a blue brush.
            hr = d2d_render_target->CreateSolidColorBrush(
                D2D1::ColorF(0x0A0A0A),
                &almost_black_brush
                );

            hr = d2d_render_target->CreateSolidColorBrush(
                D2D1::ColorF(0xEAEAEA),
                &m_pTextHeaderBrush
                );

            hr = d2d_render_target->CreateSolidColorBrush(
                D2D1::ColorF(0x3A3A3A),
                &m_pSeparatorLineCol
                );
        }
    }
}

static void onResize(int w, int h) {


}
static void onRender(HWND hwnd) {
    createDeviceResources(hwnd);
    d2d_render_target->BeginDraw();
    d2d_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
    d2d_render_target->Clear(D2D1::ColorF(0x1A1A1A ));

    D2D1_RECT_F headerRect = D2D1::RectF(
    0,
    0,
    splash_width,
    50
    );
    d2d_render_target->FillRectangle(&headerRect, header_bg_brush);
    

    d2d_render_target->DrawText(
            L"BORST EDITOR",
            ARRAYSIZE(L"BORST EDITOR") - 1,
            orbitronNormal,
            D2D1::RectF(8, 8, splash_width - 8, 50),
            almost_black_brush
            );

    d2d_render_target->DrawLine({0, 50}, {(float)splash_width, 50},  m_pSeparatorLineCol);

    d2d_render_target->EndDraw();

}

static LRESULT CALLBACK IntroSplashWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    bool wasHandled = false;

    switch (message)
    {
        case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            onResize(width, height);
        }
        result = 0;
        wasHandled = true;
        break;

        case WM_DISPLAYCHANGE:
        {
            InvalidateRect(hwnd, NULL, FALSE);
        }
        result = 0;
        wasHandled = true;
        break;

        case WM_PAINT:
        {
            onRender(hwnd);
            ValidateRect(hwnd, NULL);
        }
        result = 0;
        wasHandled = true;
        break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        result = 1;
        wasHandled = true;
        break;
    }

    if (!wasHandled)
    {
        result = DefWindowProc(hwnd, message, wParam, lParam);
    }

    return result;
}



HWND createSplahIntroWindow(HWND parentWindow, HINSTANCE hInstance) {

    createDeviceIndependentResources();
    createDWriteFont(L"../assets/Orbitron-VariableFont_wght.ttf",  &orbitronNormal, 14.0f);

    // Register the window class.
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = IntroSplashWndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = hInstance;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName  = NULL;
    wcex.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
    wcex.lpszClassName = L"SplashIntroClass";

    RegisterClassEx(&wcex);

    RECT r;
    GetWindowRect(parentWindow, &r);

    splash_width = 8 * 42;
    splash_height = 8 * 36;

    auto hwnd = CreateWindowEx(0, L"SplashIntroClass", L"SplashIntro", WS_CHILD | WS_VISIBLE,
    (r.right - r.left)/2 - splash_width/2 , (r.bottom - r.top)/2 - splash_height/2 - 100, splash_width, splash_height, parentWindow, nullptr, hInstance, nullptr);

    if (hwnd)
    {
        // Because the SetWindowPos function takes its size in pixels, we
        // obtain the window's DPI, and use it to scale the window size.
        float dpi = GetDpiForWindow(hwnd);

        SetWindowPos(
            hwnd,
            NULL,
            NULL,
            NULL,
            static_cast<int>(ceil(splash_width * dpi / 96.f)),
            static_cast<int>(ceil(splash_height * dpi / 96.f)),
            SWP_NOMOVE);
        ShowWindow(hwnd, SW_SHOWNORMAL);
        UpdateWindow(hwnd);
    }




    return hwnd;
}