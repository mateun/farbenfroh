//
// Created by mgrus on 22.07.2024.
//

#ifndef SIMPLE_KING_SPLASH_H
#define SIMPLE_KING_SPLASH_H
#include <d3d11.h>


/**
 * This holds all data needed to render the splash screen.
 * Mainly DX11 objects.
 */
struct Splash {
    ID3D11Buffer *matrixBuffer = nullptr;
    ID3D11Buffer *posBuffer = nullptr;
    ID3D11Buffer *indexBuffer = nullptr;
    ID3D11Buffer *uvBuffer = nullptr;
    ID3D11Texture2D *tex = nullptr;
    ID3D11SamplerState* samplerState = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    ID3D11RasterizerState* rs = nullptr;
    int numberIndices = 6;

};

void initSplash(Splash* splash);
void renderSplash(Splash* splash);

#endif //SIMPLE_KING_SPLASH_H
