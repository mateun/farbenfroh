//
// Created by mgrus on 25.03.2025.
//

#include "FrameBuffer.h"

#include <memory>
#include <GL/glew.h>
#include <vector>

GLsizei FrameBuffer::width() {
  return texture_->width();
}

GLsizei FrameBuffer::height() {
  return texture_->height();
}

std::shared_ptr<Texture> FrameBuffer::texture() const {
  return texture_;
}

void FrameBuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, handle_);
}

void FrameBuffer::clearDepth() {
    glClear(GL_DEPTH_BUFFER_BIT);
 }

FrameBuffer::FrameBuffer(int width, int height, bool hdr, bool additionalColorBuffer) {

    if (hdr) {
        texture_ =  Texture::createEmptyFloatTexture(width, height);
    } else {
        texture_ =  Texture::createEmptyTexture(width, height);
    }
    if (additionalColorBuffer) {
        texture2_ = Texture::createEmptyFloatTexture(width, height);
    }

}

FrameBuffer::FrameBuffer(std::shared_ptr<Texture> colorTexture, std::shared_ptr<Texture> colorTexture2) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    texture_ = colorTexture;

    // Attach the texture to the framebuffer's color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture->handle(), 0);

    if (colorTexture2) {
        texture2_ = colorTexture2;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, colorTexture2->handle(), 0);
    }

    GLuint depthRenderbuffer;
    // Step 3: Create a renderbuffer for depth attachment
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, colorTexture->width(), colorTexture->height());

    // Attach the renderbuffer to the framebuffer's depth attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

    // Step 4: Set the list of draw buffers.
    std::vector<GLenum> drawBuffers;
    drawBuffers.push_back(GL_COLOR_ATTACHMENT0);
    if (colorTexture2) {
        drawBuffers.push_back(GL_COLOR_ATTACHMENT1);
    }
    glDrawBuffers(drawBuffers.size(), drawBuffers.data());

    // Step 5: Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: Framebuffer is not complete!\n");
    } else {
        printf("Framebuffer created successfully!\n");
    }

    // Unbind the framebuffer to avoid rendering to it accidentally
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}
