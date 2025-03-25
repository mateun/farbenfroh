//
// Created by mgrus on 25.03.2025.
//

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <GL\glew.h>
#include <memory>
#include <engine/graphics/Texture.h>

/**
* Wraps an OpenGL framebuffer
*/
class FrameBuffer {

  public:
    FrameBuffer(int width, int height, bool hdr, bool additionalColorBuffer);
    explicit FrameBuffer(std::shared_ptr<Texture> colorTexture, std::shared_ptr<Texture> colorTexture2 = nullptr);

    GLsizei width();
    GLsizei height();

    GLuint handle();
    std::shared_ptr<Texture> texture() const;


    void bind();

    void clearDepth();

  private:
    GLuint handle_;
    std::shared_ptr<Texture> texture_;
    std::shared_ptr<Texture> texture2_;


};



#endif //FRAMEBUFFER_H
