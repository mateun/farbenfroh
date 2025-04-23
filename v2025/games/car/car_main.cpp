//
// Created by mgrus on 21.07.2024.
//
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include "splash.h"
#include "car.h"

using namespace DirectX;

// Globals
cargame::GameData* gameData = nullptr;

// End globals

extern stbi_uc* stbi_load(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
extern bool importModel(const std::string& file, cargame::Model* model);

namespace shaders {
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;
    ID3DBlob* vsUnlitBlob = nullptr;
    ID3DBlob* psUnlitBlob = nullptr;
    ID3D11VertexShader* vertexShaderUnlit = nullptr;
    ID3D11PixelShader* pixelShaderUnlit = nullptr;
}

namespace dx11 {
    ID3D11Device *device;
    ID3D11DeviceContext *ctx;
    IDXGISwapChain *swapChain;
    ID3D11Debug* debugger;
    ID3D11Texture2D *backBuffer;
    ID3D11RenderTargetView* rtv;
    ID3D11Texture2D* depthStencilBuffer;
    ID3D11DepthStencilView *depthStencilView;

    void clearBackbuffer(float *clearColors);
    void presentBackbuffer();
    void renderMesh(ID3D11Buffer* matrixBuffer, const DirectX::XMMATRIX &modelMatrix,
                    const DirectX::XMMATRIX &viewMatrix,
                    const DirectX::XMMATRIX &projMatrix,
                          ID3D11ShaderResourceView * srv,
                          ID3D11SamplerState * samplerState,
                          ID3D11RasterizerState* rasterizerState,
                          ID3D11Buffer* vbuf,
                          ID3D11Buffer* ibuf,
                          ID3D11Buffer* uvbuf,
                          ID3D11Buffer* nbuf,
                          int numberOfIndices,
                          XMFLOAT2 textureScale = {1, 1});
    void setViewport(int x, int y, int w, int h);
}



void dx11::setViewport(int x, int y, int w, int h) {
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.TopLeftX = x;
    vp.TopLeftY = y;
    vp.Width = w;
    vp.Height = h;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    dx11::ctx->RSSetViewports(1, &vp);
}

void dx11::presentBackbuffer() {
    dx11::swapChain->Present(0, 0);

}

void dx11::clearBackbuffer(float *clearColors) {
    ID3D11RenderTargetView* rtvs[1] = { dx11::rtv };
    dx11::ctx->OMSetRenderTargets(1, rtvs, dx11::depthStencilView);
    dx11::ctx->ClearRenderTargetView(dx11::rtv, clearColors);

    // clear our depth target as well
    dx11::ctx->ClearDepthStencilView(dx11::depthStencilView, D3D11_CLEAR_DEPTH, 1, 0);
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
                                       &dx11::device, NULL, &dx11::ctx);
    if (FAILED(result)) {
        OutputDebugString(reinterpret_cast<LPCSTR>(L"CreateDevice failed\n"));
        exit(2);
    }

    UINT ql;
    dx11::device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UINT, 8, &ql);

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
    result = dx11::device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
    IDXGIAdapter * pDXGIAdapter = nullptr;
    result = pDXGIDevice->GetAdapter(&pDXGIAdapter);
    IDXGIFactory * pIDXGIFactory = nullptr;
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);

    result = pIDXGIFactory->CreateSwapChain(dx11::device, &scdesc, &dx11::swapChain);
    if (FAILED(result)) {
        OutputDebugStringW(L"error creating swapchain\n");
        exit(1);
    }

    pIDXGIFactory->Release();
    pDXGIAdapter->Release();
    pDXGIDevice->Release();


    // Gather the debug interface
    dx11::debugger = 0;
    result = dx11::device->QueryInterface(__uuidof(ID3D11Debug), (void**)&dx11::debugger);
    if (FAILED(result)) {
        OutputDebugString("debuger creation failed\n");
        exit(1);
    }



    // Create a backbuffer
    result = dx11::swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&dx11::backBuffer);
    if (FAILED(result)) {
        OutputDebugString("backbuffer creation failed\n");
        exit(1);
    }

    // Bind the backbuffer as our render target
    result = dx11::device->CreateRenderTargetView(dx11::backBuffer, NULL, &dx11::rtv);
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

    result = dx11::device->CreateTexture2D(&td, 0, &dx11::depthStencilBuffer);
    if (FAILED(result)) {
        OutputDebugString("D S buffer creation failed\n");
        exit(1);
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dpd;
    ZeroMemory(&dpd, sizeof(dpd));
    dpd.Flags = 0;
    dpd.Format = DXGI_FORMAT_D32_FLOAT;
    dpd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    result = dx11::device->CreateDepthStencilView(dx11::depthStencilBuffer, &dpd, &dx11::depthStencilView);
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
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(dx11::device, dx11::ctx);


}

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
    HRESULT res = const_cast<ID3D11Device*>(dx11::device)->CreateTexture2D(&desc, NULL, textureTarget);
    if (FAILED(res)) {
        printf("texture creation failed\n");
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    dx11::ctx->Map(*textureTarget, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
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
    dx11::ctx->Unmap(*textureTarget, 0);
}

void compileVertexShaderWithFail(const std::wstring& vshaderFileName, ID3D10Blob** blob, ID3D11VertexShader** vertexShader) {
    ID3DBlob* errBlob = nullptr;
    HRESULT res = D3DCompileFromFile(vshaderFileName.c_str(), NULL, NULL, "VShader", "vs_5_0", 0, 0, blob, &errBlob);
    if (FAILED(res)) {
        printf("error while loading basic shader\n");

        if (errBlob) {
            printf("error: %s\n", (char*)errBlob->GetBufferPointer());
            errBlob->Release();
        }
        exit(1);
    }

    res = const_cast<ID3D11Device*>(dx11::device)->CreateVertexShader((*blob)->GetBufferPointer(), (*blob)->GetBufferSize(), NULL, vertexShader);
    if (FAILED(res)) {
        printf("failed to create vertex shader\n");
        exit(-1);
    }

}

void compilePixelShaderWithFail(const std::wstring& file, ID3D10Blob** blob, ID3D11PixelShader ** pshader) {
    ID3DBlob* errBlob = nullptr;
    HRESULT res = D3DCompileFromFile(file.c_str(), NULL, NULL, "PShader", "ps_5_0", 0, 0, blob, &errBlob);
    if (FAILED(res)) {
        printf("shader load failed\n");
        if (errBlob)
        {
            printf((char*)errBlob->GetBufferPointer());
            errBlob->Release();
        }

        if (shaders::psBlob)
            shaders::psBlob->Release();

        exit(2);
    }

    res = const_cast<ID3D11Device*>(dx11::device)->CreatePixelShader((*blob)->GetBufferPointer(), (*blob)->GetBufferSize(), NULL, pshader);
    if (FAILED(res)) {
        printf("failed to create vertex shader\n");
        exit(3);
    }
}

void initShaders() {
    compileVertexShaderWithFail(L"../games/car/shaders/basic.hlsl", &shaders::vsBlob, &shaders::vertexShader);
    compilePixelShaderWithFail(L"../games/car/shaders/basic.hlsl", &shaders::psBlob, &shaders::pixelShader);
    compileVertexShaderWithFail(L"../games/car/shaders/basic.hlsl", &shaders::vsUnlitBlob, &shaders::vertexShaderUnlit);
    compilePixelShaderWithFail(L"../games/car/shaders/basic.hlsl", &shaders::psUnlitBlob, &shaders::pixelShaderUnlit);
}

struct MatrixBufferTypeUnlit {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX proj;
};

struct MatrixBufferType {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX proj;
    XMFLOAT2 textureScale;
};


void dx11::renderMesh(ID3D11Buffer* matrixBuffer,
                      const XMMATRIX &modelMatrix,
                      const XMMATRIX &viewMatrix,
                      const XMMATRIX &projMatrix,
                      ID3D11ShaderResourceView* srv,
                      ID3D11SamplerState* samplerState,
                      ID3D11RasterizerState* rs,
                      ID3D11Buffer * vertexBuffer,
                      ID3D11Buffer * indexBuffer,
                      ID3D11Buffer * uvBuffer,
                      ID3D11Buffer * normalBuffer,
                      int numberOfIndices,
                      XMFLOAT2 textureScale
                      ) {

    D3D11_MAPPED_SUBRESOURCE bufSR;
    MatrixBufferType* dataPtr;
    HRESULT res = dx11::ctx->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufSR);
    dataPtr = (MatrixBufferType*) bufSR.pData;
    dataPtr->world = modelMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->proj = projMatrix;
    dataPtr->textureScale = textureScale;
    dx11::ctx->Unmap(matrixBuffer, 0);

    dx11::ctx->VSSetConstantBuffers(0, 1, &matrixBuffer);
    dx11::ctx->PSSetShaderResources(0, 1, &srv);
    dx11::ctx->PSSetSamplers(0, 1, &samplerState);
    dx11::ctx->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx11::ctx->RSSetState(rs);

    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;
    dx11::ctx->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    UINT uvStride = sizeof(XMFLOAT2);
    dx11::ctx->IASetVertexBuffers(1, 1, &uvBuffer, &uvStride, &offset);

    if (normalBuffer) {
        dx11::ctx->IASetVertexBuffers(2, 1, &normalBuffer, &stride, &offset);
        dx11::ctx->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }

    dx11::ctx->DrawIndexed(numberOfIndices, 0, 0);

}



// This is provided by the windows framework and should be called every frame.
extern bool processOSMessages();

void update() {
    if (keyPressed(VkKeyScan(' '))) {
        gameData->state = cargame::GameState::main_menu;
    }
}

void renderSplash() {
    float clearColor[] = {0, 0, 0.3, 1};
    dx11::clearBackbuffer(clearColor);
    renderSplash(&gameData->splash);

}

void renderMainMenu() {
    float clearColor[] = {0, 0, 0, 1};
    dx11::clearBackbuffer(clearColor);
    gameData->mainMenuScene->render();

    {
        ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
        bool p_open = true;
        if (!ImGui::Begin("Tankofant InGame Editor", &p_open)) {
            ImGui::End();
            return;
        }
        if (ImGui::Button("Play")) {
            printf("play pressed\n");
        }

        if (ImGui::Button("Pause")) {
            printf("pause pressed\n");
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
        if (ImGui::BeginTable("GameObjects", 4,
                              ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("GameObject");
            ImGui::TableSetupColumn("Data");
            ImGui::TableHeadersRow();

            // Iterate placeholder objects (all the same data)

            int index = 0;


            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::End();
    }

}

void render() {

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (gameData->state == cargame::GameState::splash) {
        renderSplash();
    }

    if (gameData->state == cargame::GameState::main_menu) {
        renderMainMenu();
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    dx11::presentBackbuffer();
}

void assignTextureToModel(const std::string& textureFile, ID3D11Texture2D* targetTexture,
                          cargame::Model* model) {

    loadTextureFromFile(textureFile, &targetTexture);
    auto res = dx11::device->CreateShaderResourceView(targetTexture, NULL, &model->srv);
    if (FAILED(res)) {
        exit(100);
    }
    D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    res = dx11::device->CreateSamplerState(&sd, &model->samplerState);
    if (FAILED(res)) {
        exit(101);
    }
}

void loadModels() {

    auto houseModel = new cargame::Model();
    auto ok = importModel("../games/car/assets/house.fbx", houseModel);
    if (!ok) {
        exit(1);
    }
    gameData->houseModel = houseModel;


    auto trackModel = new cargame::Model();
    ok = importModel("../games/car/assets/track.fbx", trackModel);
    if (!ok) {
        exit(1);
    }
    gameData->trackModel = trackModel;


    auto carModel = new cargame::Model();
    ok = importModel("../games/car/assets/simple_car2.fbx", carModel);
    if (!ok) {
        exit(1);
    }
    gameData->carModel = carModel;

    auto groundPlaneModel = new cargame::Model();
    ok = importModel("../games/car/assets/ground_plane.fbx", groundPlaneModel);
    if (!ok) {
        exit(1);
    }
    gameData->groundPlaneModel = groundPlaneModel;

    assignTextureToModel("../games/car/assets/road_asphalt.png", gameData->roadTexture, gameData->trackModel);
    assignTextureToModel("../games/car/assets/lp_colors.png", gameData->xboxTexture, gameData->houseModel);
    assignTextureToModel("../games/car/assets/lp_colors.png", gameData->xboxTexture, gameData->carModel);
    assignTextureToModel("../games/car/assets/road_asphalt.png", gameData->xboxTexture, gameData->groundPlaneModel);

}

void initRasterizerStates() {
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    rd.FillMode = D3D11_FILL_WIREFRAME;
    rd.DepthClipEnable = true;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = false;
    auto res = dx11::device->CreateRasterizerState(&rd, &gameData->rsWireFrame);
    if (FAILED(res)) {
        printf("wireframe rs failed\n");
        exit(1);
    }

    D3D11_RASTERIZER_DESC rds;
    ZeroMemory(&rds , sizeof(rds));
    rds.FillMode = D3D11_FILL_SOLID;
    rds.DepthClipEnable = true;
    rds.CullMode = D3D11_CULL_BACK;
    rds.FrontCounterClockwise = false;
    res = dx11::device->CreateRasterizerState(&rds, &gameData->rsSolid);
    if (FAILED(res)) {
        printf("wireframe rs failed\n");
        exit(1);
    }
}

void initScenes() {
    auto s = new cargame::Scene();
    auto gpn = new cargame::SceneNode();
    gpn->setModel(gameData->groundPlaneModel);
    gpn->setPosition(0, 0, 0);
    gpn->setScale(10, 1, 10);
    gpn->setRasterizerState(gameData->rsSolid);
    gpn->setTextureScale(8, 8);


    auto trackNode = new cargame::SceneNode();
    trackNode->setModel(gameData->trackModel);
    trackNode->setPosition(0, 0, 0);
    trackNode->setScale(1, 1, 1);
    trackNode->setRasterizerState(gameData->rsSolid);
    trackNode->setTextureScale(1, 1);

    auto carNode = new cargame::SceneNode();
    carNode->setModel(gameData->carModel);
    carNode->setPosition(5, 0, 1);
    carNode->setScale(1, 1, 1);
    carNode->setRasterizerState(gameData->rsSolid);
    carNode->setOrientation({0, XMConvertToRadians(-90), 0});

    auto houseNode = new cargame::SceneNode();
    houseNode->setModel(gameData->houseModel);
    houseNode->setPosition(-15, 0, 25);
    houseNode->setScale(1, 1, 1);
    houseNode->setOrientation({0, XMConvertToRadians(90), 0});
    houseNode->setRasterizerState(gameData->rsSolid);

    s->addNode(gpn);
    s->addNode(carNode);
    s->addNode(houseNode);
    s->addNode(trackNode);

    gameData->mainMenuScene = s;

}

// This gets called by the framework after the window has been created.
// If we are in activeLoop mode, the renderer is NOT ready at this point.
// If non-activeLoop, the renderer is already setup.
void gameInit(HWND hwnd, HINSTANCE hinstance) {

    initDX11(hwnd, hinstance);
    initShaders();

    gameData = new cargame::GameData();
    gameData->hwnd = hwnd;

    Splash splash;
    initSplash(&splash);
    gameData->splash = splash;

    initRasterizerStates();
    loadModels();
    initScenes();

    while (true) {
        if (!processOSMessages()) {
            break;
        }

        update();
        render();

    }

}