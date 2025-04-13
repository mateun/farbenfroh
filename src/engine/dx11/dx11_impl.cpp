//
// Created by mgrus on 13.04.2025.
//
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <string>
#include <stb_image.h>

static ID3D11Device *device;
static ID3D11DeviceContext *ctx;
static IDXGISwapChain *swapChain;
static ID3D11Debug* debugger;
static ID3D11Texture2D *backBuffer;
static ID3D11RenderTargetView* rtv;
static ID3D11Texture2D* depthStencilBuffer;
static ID3D11DepthStencilView *depthStencilView;

void loadTextureFromFile(const std::string& fileName, ID3D11Texture2D **textureTarget) {
    int imageChannels;
    int w, h;
    auto pixels = stbi_load(
            fileName.c_str(), &w, &h,
            &imageChannels,
            4);

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = w;
    desc.Height = h;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    HRESULT res = const_cast<ID3D11Device*>(device)->CreateTexture2D(&desc, NULL, textureTarget);
    if (FAILED(res)) {
        printf("texture creation failed\n");
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    ctx->Map(*textureTarget, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    BYTE* mappedData = reinterpret_cast<BYTE*>(mapped.pData);
    for (UINT i = 0; i < h; ++i) {
        memcpy(mappedData, pixels, w * 4);
        mappedData += mapped.RowPitch;
        pixels += (w * 4);
    }
    // Unused code for poking
    // bgra values directly into the image -
    // for procedural texture generation.
    /*for (UINT x = 0; x < 64; x += 4) {
    for (UINT i = 0; i < 64; ++i)
    {
    BYTE b = 0xaf;
    BYTE g = 0xbb;
    BYTE r = 0x10;
    BYTE a = 0xff;
    mappedData[x + (i * 64)] = b;
    mappedData[x + (i * 64) + 1] = g;
    mappedData[x + (i * 64) + 2] = r;
    mappedData[x + (i * 64) + 3] = a;
    }
    }*/
    ctx->Unmap(*textureTarget, 0);
}

void GetWindowClientSize(HWND hwnd, int& width, int& height)
{
    RECT rect;
    if (GetClientRect(hwnd, &rect))
    {
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }
    else
    {
        width = height = 0; // Handle error case
    }
}

void initDX11(HWND hwnd, HINSTANCE hinst) {
    int w, h;
    GetWindowClientSize(hwnd, w, h);
    D3D_FEATURE_LEVEL featureLevels =  D3D_FEATURE_LEVEL_11_1;
    HRESULT result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &featureLevels, 1, D3D11_SDK_VERSION,
                                       &device, NULL, &ctx);
    if (FAILED(result)) {
        OutputDebugString(reinterpret_cast<LPCSTR>(L"CreateDevice failed\n"));
        exit(2);
    }

    UINT ql;
    device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UINT, 8, &ql);

    DXGI_SWAP_CHAIN_DESC scdesc;
    ZeroMemory(&scdesc, sizeof(scdesc));
    scdesc.BufferCount = 1;
    scdesc.BufferDesc.Width = w;
    scdesc.BufferDesc.Height = h;
    scdesc.Windowed = true;

    scdesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    scdesc.BufferDesc.RefreshRate.Numerator = 60;
    scdesc.BufferDesc.RefreshRate.Denominator = 1;

    scdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    scdesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    scdesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

    scdesc.OutputWindow = hwnd;
    scdesc.SampleDesc.Count = 8;	// 1 sample per pixel
    scdesc.SampleDesc.Quality = 0;
    scdesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scdesc.Flags = 0;

    IDXGIDevice * pDXGIDevice = nullptr;
    result = device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
    IDXGIAdapter * pDXGIAdapter = nullptr;
    result = pDXGIDevice->GetAdapter(&pDXGIAdapter);
    IDXGIFactory * pIDXGIFactory = nullptr;
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);

    result = pIDXGIFactory->CreateSwapChain(device, &scdesc, &swapChain);
    if (FAILED(result)) {
        OutputDebugStringW(L"error creating swapchain\n");
        exit(1);
    }

    pIDXGIFactory->Release();
    pDXGIAdapter->Release();
    pDXGIDevice->Release();


    // Gather the debug interface
    debugger = 0;
    result = device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debugger);
    if (FAILED(result)) {
        OutputDebugString("debuger creation failed\n");
        exit(1);
    }



    // Create a backbuffer
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(result)) {
        OutputDebugString("backbuffer creation failed\n");
        exit(1);
    }

    // Bind the backbuffer as our render target
    result = device->CreateRenderTargetView(backBuffer, NULL, &rtv);
    if (FAILED(result)) {
        OutputDebugString("rtv creation failed\n");
        exit(1);
    }

    // Create a depth/stencil buffer
    D3D11_TEXTURE2D_DESC td;
    td.Width = w;
    td.Height = h;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_D32_FLOAT;
    td.SampleDesc.Count = 8;
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    td.CPUAccessFlags = 0;
    td.MiscFlags = 0;

    result = device->CreateTexture2D(&td, 0, &depthStencilBuffer);
    if (FAILED(result)) {
        OutputDebugString("D S buffer creation failed\n");
        exit(1);
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dpd;
    ZeroMemory(&dpd, sizeof(dpd));
    dpd.Flags = 0;
    dpd.Format = DXGI_FORMAT_D32_FLOAT;
    dpd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    result = device->CreateDepthStencilView(depthStencilBuffer, &dpd, &depthStencilView);
    if (FAILED(result)) {
        OutputDebugString("D S view creation failed\n");
        exit(1);
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Currently we just use the default from D3D11 ...
    //ID3D11DepthStencilState *m_DepthStencilState;
    //result = _device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
    /*if (FAILED(result)) {
        OutputDebugString(L"failed to set depth stencil state\n");
        exit(1);
    }*/
    //_ctx->OMSetDepthStencilState(m_DepthStencilState, 0);

    // IMGUI
    //IMGUI_CHECKVERSION();
    //ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //ImGui_ImplWin32_Init(hwnd);
    //ImGui_ImplDX11_Init(device, ctx);


}