//
// Created by mgrus on 25.03.2025.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL\glew.h>
#include <engine/graphics/Bitmap.h>
#include <memory>
#include <string>

class gru::Bitmap;

struct TextureCreationData {
    GLint internalFormat;
    GLint format;
    GLint magFilterType;
    GLint minFilterType;


};

class Texture {

  public:
    Texture();
    explicit Texture(const std::string& fileName);
    explicit Texture(GLuint existingHandle);
    explicit Texture(GLuint existingHandle, gru::Bitmap* existingBitmap);
    void bindAt(int unitIndex) const;
    [[nodiscard]] GLuint handle() const;

    uint8_t* pixels();

    static std::unique_ptr<Texture> createEmptyFloatTexture(int w, int h);
    static std::unique_ptr<Texture> createEmptyTexture(int w, int h);
    static std::unique_ptr<Texture> createTextureFromBitmap(gru::Bitmap* bm, const TextureCreationData& textureData = {GL_SRGB8_ALPHA8, GL_RGBA, GL_LINEAR, GL_LINEAR});

    GLsizei height();
    GLsizei width();

private:
    GLuint handle_;
    gru::Bitmap* bitmap_;


};




#endif //TEXTURE_H
