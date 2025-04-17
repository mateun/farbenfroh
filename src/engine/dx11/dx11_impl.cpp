//
// Created by mgrus on 13.04.2025.
//


#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <string>
#include <stb_image.h>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "dx11_api.h"

static ID3D11Device *device;
static ID3D11DeviceContext *ctx;
static IDXGISwapChain1 *swapChain;
static ID3D11Debug* debugger;
static ID3D11Texture2D *backBuffer;
static ID3D11RenderTargetView* rtv;
static ID3D11Texture2D* depthStencilBuffer;
static ID3D11DepthStencilView *depthStencilView;
static ID3D11DepthStencilState *m_DepthStencilState;

ID3D11SamplerState* dx11_createSamplerState() {
    ID3D11SamplerState* ss;
    D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    auto hr = device->CreateSamplerState(&sd, &ss);
    if (FAILED(hr)) {
        exit(101);
    }
    return ss;
}

ID3D11ShaderResourceView* dx11_createShaderResourceView(ID3D11Texture2D* texture) {
    ID3D11ShaderResourceView *srv = nullptr;
    auto res = device->CreateShaderResourceView(texture, NULL, &srv);
    if (FAILED(res)) {
        exit(100);
    }
    return srv;
}

ID3D11Texture2D * dx11_loadTextureFromFile(const std::string& fileName) {
    ID3D11Texture2D *textureTarget;
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
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = pixels;
    initialData.SysMemPitch = w * 4;
    initialData.SysMemSlicePitch = 0;

    HRESULT res = device->CreateTexture2D(&desc, &initialData, &textureTarget);
    if (FAILED(res)) {
        printf("texture creation failed\n");
        return nullptr;
    }
    return textureTarget;

    // Writing data directly to this texture procedurally.
    // D3D11_MAPPED_SUBRESOURCE mapped;
    // ctx->Map(textureTarget, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    // BYTE* mappedData = reinterpret_cast<BYTE*>(mapped.pData);
    // for (UINT i = 0; i < h; ++i) {
    //     memcpy(mappedData, pixels, w * 4);
    //     mappedData += mapped.RowPitch;
    //     pixels += (w * 4);
    // }
    // // Unused code for poking
    // // bgra values directly into the image -
    // // for procedural texture generation.
    // /*for (UINT x = 0; x < 64; x += 4) {
    // for (UINT i = 0; i < 64; ++i)
    // {
    // BYTE b = 0xaf;
    // BYTE g = 0xbb;
    // BYTE r = 0x10;
    // BYTE a = 0xff;
    // mappedData[x + (i * 64)] = b;
    // mappedData[x + (i * 64) + 1] = g;
    // mappedData[x + (i * 64) + 2] = r;
    // mappedData[x + (i * 64) + 3] = a;
    // }
    // }
    // */
    // ctx->Unmap(textureTarget, 0);


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

void dx11_drawFromIndexBuffer(ID3D11Buffer* indexBuffer, ID3D11Buffer* vertexBuffer, int vbStride, int count, int startIndex, int baseVertexLocation) {
    assert(vertexBuffer != nullptr);

    UINT stride = vbStride;
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    ctx->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->DrawIndexed(count, startIndex, baseVertexLocation);
}

void dx11_drawFromVertexBuffer(ID3D11Buffer* vertexBuffer, uint32_t stride, uint32_t offset) {
    ctx->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // TODO, fix this hardcoded number here, omg!!
    ctx->Draw(84, 0);
}

ID3D11Buffer* dx11_createIndexBuffer(std::vector<uint32_t> indices) {
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(uint32_t) * indices.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices.data();

    ID3D11Buffer* buf = nullptr;
    device->CreateBuffer(&bd, &initData, &buf);
    return buf;
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
    //ctx->OMSetRenderTargets(1, rtvs, nullptr);
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


    DXGI_SWAP_CHAIN_DESC1 scdesc1 = {};
    scdesc1.Width = w;
    scdesc1.Height = h;
    scdesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scdesc1.Stereo = FALSE;
    scdesc1.SampleDesc.Count = 1;
    scdesc1.SampleDesc.Quality = 0;
    scdesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scdesc1.BufferCount = 2;
    scdesc1.Scaling = DXGI_SCALING_NONE;
    scdesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scdesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    scdesc1.Flags = 0;


    IDXGIDevice * pDXGIDevice = nullptr;
    result = device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
    IDXGIAdapter * pDXGIAdapter = nullptr;
    result = pDXGIDevice->GetAdapter(&pDXGIAdapter);
    // IDXGIFactory * pIDXGIFactory = nullptr;
    // pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);

    IDXGIFactory2* pFactory2 = nullptr;
    auto hr= pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pFactory2);
    if (FAILED(hr)) {
        exit(5557788);
    }

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsdesc = {};
    fsdesc.Windowed = TRUE;
    result = pFactory2->CreateSwapChainForHwnd(pDXGIDevice, hwnd, &scdesc1, &fsdesc, nullptr, &swapChain);
    if (FAILED(result)) {
        printf("error creating swapchain 0x%08X\n ", result);
        exit(1);
    }

    pFactory2->Release();
    // pIDXGIFactory->Release();
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

    result = device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
    if (FAILED(result)) {
        OutputDebugString(L"failed to set depth stencil state\n");
        exit(1);
    }
    ctx->OMSetDepthStencilState(m_DepthStencilState, 0);

    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = TRUE;
    rsDesc.DepthClipEnable = TRUE;

    ID3D11RasterizerState* rasterState = nullptr;
    device->CreateRasterizerState(&rsDesc, &rasterState);
    ctx->RSSetState(rasterState);


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

ID3D11Device* dx11_device() {
    return device;
}

ID3D11DeviceContext* dx11_context() {
    return ctx;
}


ID3D11VertexShader* dx11_createVertexShaderFromByteCode(ID3DBlob* bc) {
    ID3D11VertexShader* vs = nullptr;
    auto hr = device->CreateVertexShader(bc->GetBufferPointer(),
        bc->GetBufferSize(), nullptr, &vs);

    return vs;

}

// !!!! ...................................................................
// !!!! Blender export hint: y up, -z forward (!), 0.01 scale !!!!
// !!!! ....................................................................
void dx11_loadFirstMeshFromFile(const std::string& fileName, dx11_Mesh* mesh ) {
    using namespace DirectX;

    Assimp::Importer importer;

    const auto scene = importer.ReadFile(fileName.c_str(), aiProcess_Triangulate |
                                    // aiProcess_MakeLeftHanded |
                                    // aiProcess_FlipWindingOrder |
                                    aiProcess_CalcTangentSpace);
    if (!scene) {
        printf("model import failed!\n");
        exit(1);
    }

    if (scene->mNumMeshes == 0) {
        return;
    }

    aiMatrix4x4 correction;
    aiMatrix4x4::RotationY(-AI_MATH_PI, correction);

    auto ai_mesh = scene->mMeshes[0];

    std::vector<float> interleaved_data;
    for (int v = 0; v < ai_mesh->mNumVertices; ++v) {
        aiVector3D vertex = ai_mesh->mVertices[v] ;
        aiVector3D texCoord = ai_mesh->mTextureCoords[0][v];
        aiVector3D normal = ai_mesh->mNormals[v];

        vertex *= correction;
        normal *= correction;

        interleaved_data.push_back(vertex.x);
        interleaved_data.push_back(vertex.y);
        interleaved_data.push_back(-vertex.z);
        interleaved_data.push_back(texCoord.x);
        interleaved_data.push_back(1-texCoord.y);
        interleaved_data.push_back(-normal.x);
        interleaved_data.push_back(normal.y);
        interleaved_data.push_back(normal.z);


    }

    for (int f = 0; f < ai_mesh->mNumFaces; ++f) {
        aiFace face = ai_mesh->mFaces[f];
        mesh->indices.push_back({ face.mIndices[0] });
        mesh->indices.push_back({ face.mIndices[1] });
        mesh->indices.push_back({ face.mIndices[2] });
    }

    // Positions, uvs and normals all go into 1 interleaved buffer.
    // Indices get a separate buffer.
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(float) * interleaved_data.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA interleaved;
    interleaved.pSysMem = interleaved_data.data();
    interleaved.SysMemPitch = 0;
    interleaved.SysMemSlicePitch = 0;
    auto hr = device->CreateBuffer(&bd, &interleaved, &mesh->pos_uv_normal_buffer);
    if (FAILED(hr)) {
        exit (007);
    }

    // Don't use separate buffers
    // TODO make this configurable as a parameter?
    // // UVs
    // ZeroMemory(&bd, sizeof(bd));
    // bd.Usage = D3D11_USAGE_DEFAULT;
    // bd.ByteWidth = sizeof(XMFLOAT2) * mesh->uvs.size();
    // bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // D3D11_SUBRESOURCE_DATA uvData;
    // uvData.pSysMem = mesh->uvs.data();
    // uvData.SysMemPitch = 0;
    // uvData.SysMemSlicePitch = 0;
    // device->CreateBuffer(&bd, &uvData, &mesh->uvBuffer);
    //
    // // Normals
    // ZeroMemory(&bd, sizeof(bd));
    // bd.Usage = D3D11_USAGE_DEFAULT;
    // bd.ByteWidth = sizeof(XMFLOAT3) * mesh->normals.size();
    // bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // D3D11_SUBRESOURCE_DATA normalData;
    // normalData.pSysMem = mesh->normals.data();
    // normalData.SysMemPitch = 0;
    // normalData.SysMemSlicePitch = 0;
    // device->CreateBuffer(&bd, &normalData, &mesh->normalBuffer);

    // Indices
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(unsigned int) * mesh->indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = mesh->indices.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    device->CreateBuffer(&ibd, &indexData, &mesh->indexBuffer);

    // This does not belong into the import here
    // ctx->VSSetShader(shaders::vertexShader, 0, 0);
    // ctx->PSSetShader(shaders::pixelShader, 0, 0);
    //
    // ctx->IASetInputLayout(inputLayout);

    // UINT uvstride = sizeof(XMFLOAT2);
    // UINT stride = sizeof(XMFLOAT3);
    // UINT offset = 0;
    // ctx->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
    // ctx->IASetVertexBuffers(1, 1, &mesh->uvBuffer, &uvstride, &offset);
    // ctx->IASetVertexBuffers(2, 1, &mesh->normalBuffer, &stride, &offset);





}

void dx11_onResize(int w, int h) {
    if (!ctx) return;
    assert(w > 0 && h > 0);
    printf("Trying ResizeBuffers to %dx%d\n", w, h);

    ctx->OMSetRenderTargets(0, nullptr, nullptr);


    if (rtv) {
        rtv->Release(); rtv = nullptr;
    }
    if (depthStencilView) {
        depthStencilView->Release(); depthStencilView = nullptr;
    }
    if (depthStencilBuffer) {
        depthStencilBuffer->Release(); depthStencilBuffer = nullptr;
    }

    if (backBuffer) { backBuffer->Release(); backBuffer = nullptr; }

    auto hr = swapChain->ResizeBuffers(
        0,        // buffer count (0 = reuse)
        w,
        h,
        DXGI_FORMAT_UNKNOWN, // reuse existing format
        0
    );

    if (FAILED(hr)) {
        printf("ResizeBuffers failed! HRESULT: 0x%08X\n", hr);
    }
    // Get backbuffer
    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    D3D11_TEXTURE2D_DESC actualDesc;
    backBuffer->GetDesc(&actualDesc);
    printf("Backbuffer: %d x %d\n", actualDesc.Width, actualDesc.Height);
    hr = device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    backBuffer->Release();
    if (FAILED(hr)) {
        exit(11001);
    }

    // Create new depth stencil
    D3D11_TEXTURE2D_DESC dsDesc = {};
    dsDesc.Width = w;
    dsDesc.Height = h;
    dsDesc.MipLevels = 1;
    dsDesc.ArraySize = 1;
    dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsDesc.SampleDesc.Count = 1;
    dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = device->CreateTexture2D(&dsDesc, nullptr, &depthStencilBuffer);
    if (FAILED(hr)) {
        exit(11002);
    }
    device->CreateDepthStencilView(depthStencilBuffer, nullptr, &depthStencilView);
    ctx->OMSetRenderTargets(1, &rtv, depthStencilView);
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
