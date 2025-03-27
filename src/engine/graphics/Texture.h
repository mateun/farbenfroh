//
// Created by mgrus on 25.03.2025.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL\glew.h>
#include <memory>
#include <string>

class Bitmap;

class Texture {

  public:
    Texture();
    explicit Texture(const std::string& fileName);
    explicit Texture(GLuint existingHandle);
    explicit Texture(GLuint existingHandle, Bitmap* existingBitmap);
    void bindAt(int unitIndex) const;
    [[nodiscard]] GLuint handle() const;

    uint8_t* pixels();

    static std::unique_ptr<Texture> createEmptyFloatTexture(int w, int h);
    static std::unique_ptr<Texture> createEmptyTexture(int w, int h);
    static std::unique_ptr<Texture> createTextureFromBitmap(Bitmap* bm, GLint internalFormat = GL_SRGB8_ALPHA8, GLenum format = GL_RGBA);

    GLsizei height();
    GLsizei width();

private:
    GLuint handle_;
    Bitmap* bitmap_;


};




#endif //TEXTURE_H
