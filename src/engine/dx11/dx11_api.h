//
// Created by mgrus on 16.04.2025.
//

#ifndef DX11_API_H
#define DX11_API_H

#include <d3d11.h>
#include <glm/glm.hpp>


void dx11_init(HWND hwnd);
void dx11_presentBackbuffer();
void dx11_clearBackbuffer(glm::vec4 clearColors);
void dx11_drawFromVertexBuffer(ID3D11Buffer* vertexBuffer, uint32_t stride, uint32_t offset);
void dx11_drawFromIndexBuffer(ID3D11Buffer* indexBuffer, ID3D11Buffer* vertexBuffer, int vbStride, int count, int startIndex, int baseVertexLocation);
void dx11_createInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout, UINT layoutCount, ID3DBlob* vsBlob, ID3D11InputLayout** outLayout);
void dx11_setPixelShaderConstantBuffer(int slot, int num, ID3D11Buffer* buffers);
void dx11_setVertexShaderConstantBuffer(int slot, int num, ID3D11Buffer* buffers);
void dx11_setShaderAndInputLayout(ID3D11InputLayout* inputLayout, ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader);
ID3D11Buffer* dx11_createVertexBuffer(std::vector<glm::vec3> vertices);
ID3D11Buffer* dx11_createIndexBuffer(std::vector<uint32_t> indices);
ID3D11Device* dx11_device();    // not encouraged, but ultra low level direct access to the device if needed.
ID3D11DeviceContext* dx11_context(); // not engouraged, but if needed, access to the device context here.

bool dx11_compileShader(const std::wstring& filename, ID3DBlob** shaderByteCode, const std::string& entryPoint, const std::string& shaderTargetVersion);

ID3D11VertexShader* dx11_createVertexShaderFromByteCode(ID3DBlob* bc);
ID3D11PixelShader* dx11_createPixelShaderFromByteCode(ID3DBlob* bc);
//ID3D11VertexShader* dx11_createVertexShaderFromByteCode(ID3DBlob* bc);





#endif //DX11_API_H
