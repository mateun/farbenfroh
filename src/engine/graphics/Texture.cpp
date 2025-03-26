//
// Created by mgrus on 25.03.2025.
//

#include "Texture.h"
#include <GL\glew.h>
#include <engine/graphics/Bitmap.h>
#include <memory>

#include "ErrorHandling.h"

GLuint Texture::handle() const {
  return handle_;
}

Texture::Texture() {

}

Texture::Texture(const std::string &fileName) {
  bitmap_ = new Bitmap(fileName.c_str());
  glGenTextures(1, &handle_);
  glBindTexture(GL_TEXTURE_2D, handle_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_SRGB8_ALPHA8,
               bitmap_->width,
               bitmap_->height,
               0,
               GL_RGBA,
               GL_UNSIGNED_BYTE,
               bitmap_->pixels);

  glGenerateMipmap(GL_TEXTURE_2D);

  // Set aniso filtering
  {
    if (!glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
      throw std::runtime_error("Anisotropic filtering not supported!");
    }

    GLfloat maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    printf("maxAniso: %f\n", maxAniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);

  }


}

Texture::Texture(GLuint existingHandle) : handle_(existingHandle) {
}

Texture::Texture(GLuint existingHandle, Bitmap *existingBitmap) : handle_(existingHandle) , bitmap_(existingBitmap) {
}

void Texture::bindAt(int unitIndex) const {
  glActiveTexture(GL_TEXTURE0 + unitIndex);
  glBindTexture(GL_TEXTURE_2D, handle_);
  GL_ERROR_EXIT(980);
}


uint8_t * Texture::pixels() {
  return bitmap_->pixels;
}

GLsizei Texture::height() {
  return bitmap_->height;
}

GLsizei Texture::width() {
  return bitmap_->width;
}


std::unique_ptr<Texture> Texture::createTextureFromBitmap(Bitmap *bm) {
  GLuint handle;
  glGenTextures(1, &handle);

  glBindTexture(GL_TEXTURE_2D, handle);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_SRGB8_ALPHA8,
               bm->width, bm->height,
               0,
               GL_RGBA,
               GL_UNSIGNED_BYTE, bm->pixels);

  return std::make_unique<Texture>(handle, bm);
}


std::unique_ptr<Texture> Texture::createEmptyTexture(int w, int h) {
  auto pixels = (uint8_t *) _aligned_malloc(w*h*4, 32);
  auto bm = new Bitmap();
  bm->width = w;
  bm->height = h;
  bm->pixels = pixels;
  return createTextureFromBitmap(bm);
}

std::unique_ptr<Texture> Texture::createEmptyFloatTexture(int w, int h) {
    auto pixels = (uint8_t *) _aligned_malloc(w*h*4, 32);
    auto bm = new Bitmap();
    bm->width = w;
    bm->height = h;
    bm->pixels = pixels;
    GLuint handle;

    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // glTexImage2D(GL_TEXTURE_2D,
    //              0,
    //              GL_RGB32F,
    //              bm->width, bm->height,
    //              0,
    //              GL_RGBA,
    //              GL_UNSIGNED_BYTE, bm->pixels);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, w, h);

    return std::make_unique<Texture>(handle, bm);


}
