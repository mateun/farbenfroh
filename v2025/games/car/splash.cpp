


void initSplash(Splash *splash) {
    // Input format for our quad
    D3D11_INPUT_ELEMENT_DESC ied[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            //{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// same slot, but 12 bytes after the pos
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},        // other slot (buffer), starting at 0

    };
    ID3D11InputLayout *inputLayout;
    const_cast<ID3D11Device *>(dx11::device)->CreateInputLayout(ied, 2, shaders::vsUnlitBlob->GetBufferPointer(),
                                                                shaders::vsUnlitBlob->GetBufferSize(), &inputLayout);

    loadTextureFromFile("../games/car/assets/title_image.png", &splash->tex);

    if (splash->tex) {
        dx11::device->CreateShaderResourceView(splash->tex, NULL, &splash->srv);

        D3D11_SAMPLER_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.MinLOD = 0;
        sd.MaxLOD = D3D11_FLOAT32_MAX;

        dx11::device->CreateSamplerState(&sd, &splash->samplerState);

    }

    std::vector<DirectX::XMFLOAT3> mesh;
    mesh.push_back({-0.5, 0.5, 0});
    mesh.push_back({0.5, -.5, 0});
    mesh.push_back({-0.5, -.5, 0});
    mesh.push_back({0.5, .5, 0});
    std::vector<DirectX::XMFLOAT2> uvs;
    uvs.push_back({0, 1});
    uvs.push_back({1, 0});
    uvs.push_back({0, 0});
    uvs.push_back({1, 1});
    std::vector<UINT> indices;
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(3);
    indices.push_back(1);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(XMFLOAT3) * mesh.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    dx11::device->CreateBuffer(&bd, NULL, &splash->posBuffer);

    D3D11_MAPPED_SUBRESOURCE ms;
    dx11::ctx->Map(splash->posBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, mesh.data(), sizeof(XMFLOAT3) * mesh.size());
    dx11::ctx->Unmap(splash->posBuffer, NULL);


    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(XMFLOAT2) * uvs.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA uvData;
    uvData.pSysMem = uvs.data();
    uvData.SysMemPitch = 0;
    uvData.SysMemSlicePitch = 0;
    dx11::device->CreateBuffer(&bd, &uvData, &splash->uvBuffer);

    // Indices
    //unsigned int indices[] = { 0, 1, 2, 0, 3, 1 };
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(unsigned int) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    HRESULT res = dx11::device->CreateBuffer(&ibd, &indexData, &splash->indexBuffer);
    dx11::ctx->IASetIndexBuffer(splash->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    dx11::ctx->VSSetShader(shaders::vertexShader, 0, 0);
    dx11::ctx->PSSetShader(shaders::pixelShader, 0, 0);

    dx11::ctx->IASetInputLayout(inputLayout);
    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;
    dx11::ctx->IASetVertexBuffers(0, 1, &splash->posBuffer, &stride, &offset);
    UINT uvStride = sizeof(XMFLOAT2);
    dx11::ctx->IASetVertexBuffers(1, 1, &splash->uvBuffer, &uvStride, &offset);

    // Handle mvp matrices
    D3D11_BUFFER_DESC mbd;
    ZeroMemory(&mbd, sizeof(mbd));
    mbd.Usage = D3D11_USAGE_DYNAMIC;
    mbd.ByteWidth = sizeof(MatrixBufferType);
    mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    mbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    res = dx11::device->CreateBuffer(&mbd, nullptr, &splash->matrixBuffer);
    if (FAILED(res)) {
        printf("matrix constant buffer creation failed\n");
        exit(1);
    }

    // Setting the rasterizer state

    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    rd.FillMode = D3D11_FILL_SOLID;
    rd.DepthClipEnable = true;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = false;
    res = dx11::device->CreateRasterizerState(&rd, &splash->rs);
    if (FAILED(res)) {
        printf("wireframe rs failed\n");
        exit(1);
    }


}

void renderSplash(Splash *splash) {
    float ar = 1280.f / 720.0f;
    XMMATRIX projMatSplash = DirectX::XMMatrixOrthographicLH(2, 2 / ar, 0.1, 100);
    projMatSplash = XMMatrixTranspose(projMatSplash);

    auto modelMat = DirectX::XMMatrixScaling(1, 1, 1);
    modelMat = XMMatrixMultiply(modelMat, DirectX::XMMatrixTranslation(-0, -0, 0.2));
    modelMat = XMMatrixTranspose(modelMat);

    int w, h;
    GetWindowClientSize(gameData->hwnd, w, h);
    dx11::setViewport(0, 0, w, h);
    dx11::renderMesh(splash->matrixBuffer, modelMat, XMMatrixIdentity(),
                     projMatSplash, splash->srv, splash->samplerState, splash->rs,
                     splash->posBuffer, splash->indexBuffer, splash->uvBuffer, nullptr, 6);

}
