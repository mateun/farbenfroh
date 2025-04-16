//
// Created by mgrus on 15.04.2025.
//
#include <cmath>
#include <d3d11_1.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <d2d1helper.h>
#include <d2d1svg.h>

#include <functional>
#include <shlwapi.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "fonts.h"

// Private fileglobals
static ID2D1Factory2* d2d_factory;
static ID2D1DeviceContext* d2d_context;
static ID2D1DeviceContext5* d2d_context5;
static IDXGISwapChain1* swap_chain;
static ID2D1Bitmap1* d2d_target_bitmap;
static ID2D1SolidColorBrush* shared_brush;
static ID3D11Device* d3d_device;
static ID3D11DeviceContext* d3d_context;

void paint2d_clear(glm::vec4 color) {
    d2d_context5->Clear({color.r / 255.0f, color.g/ 255.0f, color.b / 255.0f, color.a / 255.0f});
}

void paint2d_begin_draw() {
    d2d_context5->BeginDraw();
}

void paint2d_end_draw() {
    HRESULT hr = d2d_context5->EndDraw();
    if (FAILED(hr)) {
        exit(99997);
    }
    swap_chain->Present(1, 0);
}

void paint2d_resize(int w, int h) {
    if (!swap_chain || !d2d_context5) return;

    // 1. Resize the DXGI swapchain buffers
    swap_chain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);

    // 2. Get new backbuffer surface
    IDXGISurface* dxgiBackbuffer;
    HRESULT hr = swap_chain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackbuffer));
    if (FAILED(hr)) {
        // Handle error...
        return;
    }

    // 3. Create new bitmap target
    D2D1_BITMAP_PROPERTIES1 bmpProps = {};
    bmpProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    bmpProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    bmpProps.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    bmpProps.dpiX = 96.0f;
    bmpProps.dpiY = 96.0f;

    ID2D1Bitmap1* newTarget;
    hr = d2d_context5->CreateBitmapFromDxgiSurface(dxgiBackbuffer, &bmpProps, &newTarget);
    if (FAILED(hr)) {
        // Handle error...
        return;
    }

    // 4. Bind new target to context
    d2d_context5->SetTarget(newTarget);
    d2d_target_bitmap = newTarget; // update your global/shared ref
}

void paint2d_init(HWND hwnd) {

    // Set up d3d, at least minmal, so we can use svg in d2d:
    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_WARP, // ← software rendering only
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &d3d_device,
        nullptr,
        &d3d_context);

    IDXGIDevice* dxgiDevice;
    HRESULT hr = d3d_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
    if (FAILED(hr)) {
        exit(1001);
    }

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory2), nullptr, (void**) &d2d_factory);
    if (FAILED(hr)) {
      exit(3336);
    }

    ID2D1Device* d2dDevice;
    d2d_factory->CreateDevice(dxgiDevice, &d2dDevice);

    d2dDevice->CreateDeviceContext(
        D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
        &d2d_context
    );
    d2d_context->QueryInterface(__uuidof(ID2D1DeviceContext5), (void**) &d2d_context5);

    // Get DXGI adapter + factory
    IDXGIAdapter* dxgiAdapter;
    dxgiDevice->GetAdapter(&dxgiAdapter);

    IDXGIFactory2* dxgiFactory;
    dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

    // Describe swapchain
    RECT rc;
    GetWindowRect(hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top);
    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    scDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.BufferCount = 2;
    scDesc.SampleDesc.Count = 1;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    scDesc.Width = size.width;
    scDesc.Height = size.height;

    dxgiFactory->CreateSwapChainForHwnd(
        d3d_device, hwnd, &scDesc,
        nullptr, nullptr, &swap_chain
    );

    // Get backbuffer surface
    IDXGISurface* dxgiBackbuffer;
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackbuffer));

    // Create D2D bitmap target from backbuffer
    D2D1_BITMAP_PROPERTIES1 bmpProps = {};
    bmpProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    bmpProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    bmpProps.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    bmpProps.dpiX = 96.0f;
    bmpProps.dpiY = 96.0f;


    d2d_context5->CreateBitmapFromDxgiSurface(
        dxgiBackbuffer,
        &bmpProps,
        &d2d_target_bitmap
    );

    // Set the bitmap as target
    d2d_context5->SetTarget(d2d_target_bitmap);

    d2d_context5->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    //d2d_context5->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

     hr = d2d_context5->CreateSolidColorBrush(
               D2D1::ColorF(0xC7FF00),
               &shared_brush
               );

    if (FAILED(hr)) {
        exit(445511);
    }


}

void paint2d_draw_filled_rect(glm::vec2 position, glm::vec2 size, glm::vec4 color) {
    shared_brush->SetColor({color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f});
    D2D1_RECT_F rect;
    rect.left = position.x;
    rect.top = position.y;
    rect.right = position.x + size.x;
    rect.bottom = position.y + size.y;

   // d2d_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
    d2d_context5->FillRectangle(rect, shared_brush);
}

void paint2d_draw_custom(std::function<void(ID2D1RenderTarget*, ID2D1SolidColorBrush*)> callback, glm::vec4 color) {
    shared_brush->SetColor({color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f});

    callback(d2d_context5, shared_brush);

}

void paint2d_create_svg(const std::wstring& filename, ID2D1SvgDocument **outSvg) {
    IStream* stream;
    HRESULT hr = SHCreateStreamOnFileEx(
        filename.c_str(),
        STGM_READ | STGM_SHARE_DENY_WRITE,
        FILE_ATTRIBUTE_NORMAL,
        FALSE,
        nullptr,
        &stream
    );

    if (FAILED(hr)) {
        exit(445512);
    }

    hr = d2d_context5->CreateSvgDocument(stream, D2D1::SizeF(24, 24), outSvg);
    if (FAILED(hr)) {
        exit(445514);
    }

}

void paint2d_draw_svg(ID2D1SvgDocument* svg, D2D1_RECT_F bounds) {
    if (!svg) return;

    D2D1_SIZE_F docSize = svg->GetViewportSize();
    if (docSize.width <= 0.1f || docSize.height <= 0.1f) {
        printf("[SVG] Invalid viewport size\n");
        return;
    }

    // auto transform = D2D1::Matrix3x2F::Scale(64, 64) *
    //                  D2D1::Matrix3x2F::Translation(200, 300);
    // d2d_context5->SetTransform(transform);
    d2d_context5->SetTransform(D2D1::Matrix3x2F::Translation(bounds.left, bounds.top));
    d2d_context5->DrawSvgDocument(svg);
    d2d_context5->SetTransform(D2D1::Matrix3x2F::Identity());
}

void paint2d_draw_text(const std::wstring& text, glm::vec2 position, glm::vec2 size, glm::vec4 color, IDWriteTextFormat* text_format) {
    shared_brush->SetColor({color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f});

    float pxX = std::floor(position.x + 0.5f);
    float pxY = std::floor(position.y + 0.5f);
    d2d_context5->DrawText(
           text.c_str(),
           text.size(),
           text_format,
           D2D1::RectF(pxX, pxY, position.x + size.x, position.y + size.y),
           shared_brush
           );

}
