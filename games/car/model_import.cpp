//
// Created by mgrus on 22.07.2024.
//

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <DirectXMath.h>
#include "car.h"

using namespace DirectX;

// Blender export hint: y up, -z forward (!), 0.01 scale
bool importModel(const std::string& file, cargame::Model* model) {

    Assimp::Importer importer;

    const auto scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder);
    if (!scene) {
        printf("model import failed!\n");
        exit(1);
    }

    unsigned int numMeshes = scene->mRootNode->mChildren[0]->mNumMeshes;
    for (int i = 0; i < numMeshes; ++i) {
        auto idx = scene->mRootNode->mChildren[0]->mMeshes[i];
        aiMesh* mesh = scene->mMeshes[i];
        auto numUVChannels = mesh->GetNumUVChannels();
        auto hasTextureCoords = mesh->HasTextureCoords(0);
        for (int v = 0; v < mesh->mNumVertices; ++v) {
            aiVector3D vertex = mesh->mVertices[v];
            aiVector3D texCoord = mesh->mTextureCoords[0][v];
            aiVector3D normal = mesh->mNormals[v];

            model->positions.push_back({ vertex.x, vertex.y, vertex.z });
            model->uvs.push_back({ texCoord.x, texCoord.y });
            model->normals.push_back({normal.x, normal.y, normal.z});

        }

        for (int f = 0; f < mesh->mNumFaces; ++f) {
            aiFace face = mesh->mFaces[f];
            model->indices.push_back({ face.mIndices[0] });
            model->indices.push_back({ face.mIndices[1] });
            model->indices.push_back({ face.mIndices[2] });
        }
    }

    D3D11_INPUT_ELEMENT_DESC ied[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            //{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// same slot, but 12 bytes after the pos
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},        // other slot (buffer), starting at 0
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},

    };
    ID3D11InputLayout *inputLayout;
    auto res = const_cast<ID3D11Device *>(dx11::device)->CreateInputLayout(ied, 3, shaders::vsBlob->GetBufferPointer(),
                                                                shaders::vsBlob->GetBufferSize(), &inputLayout);
    if (FAILED(res)) {
        exit(2);
    }


    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(XMFLOAT3) * model->positions.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    dx11::device->CreateBuffer(&bd, NULL, &model->vertexBuffer);

    // TODO : remove this, do we even need this??
    D3D11_MAPPED_SUBRESOURCE ms;
    dx11::ctx->Map(model->vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, model->positions.data(), sizeof(XMFLOAT3) * model->positions.size());
    dx11::ctx->Unmap(model->vertexBuffer, NULL);


    // UVs
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(XMFLOAT2) * model->uvs.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA uvData;
    uvData.pSysMem = model->uvs.data();
    uvData.SysMemPitch = 0;
    uvData.SysMemSlicePitch = 0;
    dx11::device->CreateBuffer(&bd, &uvData, &model->uvBuffer);

    // Normals
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(XMFLOAT3) * model->normals.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA normalData;
    normalData.pSysMem = model->normals.data();
    normalData.SysMemPitch = 0;
    normalData.SysMemSlicePitch = 0;
    dx11::device->CreateBuffer(&bd, &normalData, &model->normalBuffer);

    // Indices
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(unsigned int) * model->indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = model->indices.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    res = dx11::device->CreateBuffer(&ibd, &indexData, &model->indexBuffer);

    dx11::ctx->VSSetShader(shaders::vertexShader, 0, 0);
    dx11::ctx->PSSetShader(shaders::pixelShader, 0, 0);

    dx11::ctx->IASetInputLayout(inputLayout);

    UINT uvstride = sizeof(XMFLOAT2);
    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;
    dx11::ctx->IASetVertexBuffers(0, 1, &model->vertexBuffer, &stride, &offset);
    dx11::ctx->IASetVertexBuffers(1, 1, &model->uvBuffer, &uvstride, &offset);
    dx11::ctx->IASetVertexBuffers(2, 1, &model->normalBuffer, &stride, &offset);

    // Handle mvp matrices
    D3D11_BUFFER_DESC mbd;
    ZeroMemory(&mbd, sizeof(mbd));
    mbd.Usage = D3D11_USAGE_DYNAMIC;
    mbd.ByteWidth = sizeof(MatrixBufferType);
    mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    mbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    res = dx11::device->CreateBuffer(&mbd, nullptr, &model->matrixBuffer);
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
    res = dx11::device->CreateRasterizerState(&rd, &model->rs);
    if (FAILED(res)) {
        printf("wireframe rs failed\n");
        exit(1);
    }

    return true;
}


