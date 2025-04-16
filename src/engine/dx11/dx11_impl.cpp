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
#include <vector>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

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

void dx11_presentBackbuffer() {
    auto hr = swapChain->Present(0, 0);
    if (FAILED(hr)) {
        exit(999);
    }

}

void dx11_drawFromVertexBuffer(ID3D11Buffer* vertexBuffer, uint32_t stride, uint32_t offset) {
    ctx->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    ctx->Draw(84, 0); // 84 vertices → 42 lines
}

ID3D11Buffer* dx11_createVertexBuffer(std::vector<glm::vec3> vertices) {
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(glm::vec3) * vertices.size(); // 84 from above
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.data();

    ID3D11Buffer* vertexBuffer = nullptr;
    device->CreateBuffer(&bd, &initData, &vertexBuffer);
    return vertexBuffer;
}

void dx11_clearBackbuffer(glm::vec4 clearColors) {
    ID3D11RenderTargetView* rtvs[1] = { rtv };
    ctx->OMSetRenderTargets(1, rtvs, depthStencilView);
    ctx->ClearRenderTargetView(rtv, glm::value_ptr(clearColors));

    // clear our depth target as well
    ctx->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1, 0);
}

void dx11_init(HWND hwnd) {
    int w, h;
    GetWindowClientSize(hwnd, w, h);
    D3D_FEATURE_LEVEL featureLevels =  D3D_FEATURE_LEVEL_11_1;
    HRESULT result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &featureLevels, 1, D3D11_SDK_VERSION,
                                       &device, NULL, &ctx);
    if (FAILED(result)) {
        OutputDebugString((L"CreateDevice failed\n"));
        exit(2);
    }

    UINT ql;
    device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 8, &ql);


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

        // fallback to no MSAA
        scdesc.SampleDesc.Count = 1;
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
        OutputDebugString(L"debuger creation failed\n");
        exit(1);
    }



    // Create a backbuffer
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(result)) {
        OutputDebugString(L"backbuffer creation failed\n");
        exit(1);
    }

    // Bind the backbuffer as our render target
    result = device->CreateRenderTargetView(backBuffer, NULL, &rtv);
    if (FAILED(result)) {
        OutputDebugString(L"rtv creation failed\n");
        exit(1);
    }

    // Create a depth/stencil buffer
    D3D11_TEXTURE2D_DESC td;
    td.Width = w;
    td.Height = h;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_D32_FLOAT;
    td.SampleDesc.Count = 1;
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    td.CPUAccessFlags = 0;
    td.MiscFlags = 0;

    result = device->CreateTexture2D(&td, 0, &depthStencilBuffer);
    if (FAILED(result)) {
        OutputDebugString(L"D S buffer creation failed\n");
        exit(1);
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dpd;
    ZeroMemory(&dpd, sizeof(dpd));
    dpd.Flags = 0;
    dpd.Format = DXGI_FORMAT_D32_FLOAT;
    dpd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    result = device->CreateDepthStencilView(depthStencilBuffer, &dpd, &depthStencilView);
    if (FAILED(result)) {
        OutputDebugString(L"D S view creation failed\n");
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

void dx11_setShaderAndInputLayout(ID3D11InputLayout* inputLayout, ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader) {
    ctx->IASetInputLayout(inputLayout);
    ctx->VSSetShader(vertexShader, nullptr, 0);
    ctx->PSSetShader(pixelShader, nullptr, 0);
}

void dx11_setPixelShaderConstantBuffer(int slot, int num, ID3D11Buffer* buffers) {
    ctx->PSSetConstantBuffers(1, 1, &buffers);
}

void dx11_setVertexShaderConstantBuffer(int slot, int num, ID3D11Buffer* buffers) {
    ctx->VSSetConstantBuffers(slot, num, &buffers);


}

void dx11_createInputLayout(
    const D3D11_INPUT_ELEMENT_DESC* layout,
    UINT layoutCount,
    ID3DBlob* vsBlob,
    ID3D11InputLayout** outLayout
) {
    device->CreateInputLayout(layout, layoutCount, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), outLayout);
}



void dx11_createVertexShaderFromByteCode(ID3DBlob* bc, ID3D11VertexShader** outVS) {
    assert(device != nullptr);

    auto hr = device->CreateVertexShader(bc->GetBufferPointer(),
        bc->GetBufferSize(), nullptr, outVS);

}

ID3D11PixelShader* dx11_createPixelShaderFromByteCode(ID3DBlob* bc) {
    ID3D11PixelShader* ps = nullptr;
    auto hr = device->CreatePixelShader(bc->GetBufferPointer(),
        bc->GetBufferSize(), nullptr, &ps);
    if (SUCCEEDED(hr)) return ps;
    return nullptr;
}

bool dx11_compileShader(const std::wstring& filename, ID3DBlob** shaderByteCode, const std::string& entryPoint, const std::string& shaderTargetVersion)
{
    if (GetFileAttributesW(filename.c_str()) == INVALID_FILE_ATTRIBUTES) {
        printf("Shader file not found: %ls\n", filename.c_str());
        return false;
    }

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* errorBlob;
    HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr,
         entryPoint.c_str(), shaderTargetVersion.c_str() , flags, 0, shaderByteCode, &errorBlob);


    if (FAILED(hr)) {
        if (errorBlob) {
            std::string errorMsg((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
            OutputDebugStringA(errorMsg.c_str());
            printf("[Shader Compilation Error]: %s\n", errorMsg.c_str());
            errorBlob->Release();
        } else {
            printf("[Shader Compilation Error]: Unknown error (no error blob)\n");
        }
        return false;
    }
    return true;

}
