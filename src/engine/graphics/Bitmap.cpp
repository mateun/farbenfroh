//
// Created by mgrus on 26.03.2025.
//

#include "Bitmap.h"
#include <Windows.h>
#include <stdexcept>
#include <cstdio>

#include <io.h>

#include <cstdlib>

#include <fcntl.h>
#include <cinttypes>
#include <filesystem>
#include <engine/io/FileHelper.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>



Bitmap::Bitmap() {
}


Bitmap::Bitmap(const std::string &fileName) {
            auto f = fileName.c_str();
            if (FileHelper::isPngFile(fileName) || FileHelper::isJpgFile(fileName)) {
                int imageChannels;
                int w, h;
                auto pixels = stbi_load(f, &w, &h,
                        &imageChannels,
                        4);
                pixels = pixels;
                width = w;
                height = h;
            }
            else {
                int file = -1;

                _sopen_s( &file, f, _O_RDONLY, _SH_DENYNO, 0 );
                if (file == -1) {
                    throw std::runtime_error("Unable to open file");
                }
                _lseek(file, 0, SEEK_SET);

                BITMAP_FILE bmf;
                _read(file, &bmf.bmfileHeader, sizeof(BITMAPFILEHEADER));

                if (bmf.bmfileHeader.bfType != 0x4D42) {
                    _lclose(file);
                    exit(2);
                }
                _read(file, &bmf.bminfoHeader, sizeof(BITMAPINFOHEADER));
                if (bmf.bminfoHeader.biBitCount != 32) {
                    _lclose(file);
                    exit(3);
                }

                _lseek(file, bmf.bmfileHeader.bfOffBits, SEEK_SET);
                pixels = (uint8_t*) malloc(bmf.bminfoHeader.biSizeImage);
                _read(file, pixels, bmf.bminfoHeader.biSizeImage);
                _close(file);
                width = bmf.bminfoHeader.biWidth;
                height = bmf.bminfoHeader.biHeight;
            }

    }

