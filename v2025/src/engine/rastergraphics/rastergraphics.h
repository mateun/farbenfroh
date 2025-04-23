//
// Created by mgrus on 21.02.2025.
//

#ifndef RASTERGRAPHICS_H
#define RASTERGRAPHICS_H

#include <span>
#include <glm\glm.hpp>

enum class PixelFormat {
  RGBA8,
  BGRA8,
  RGBA32F,
  BGRA32F,
};

class Texture;

/**
* This class holds an array of memory which is mapped to a texture or similar.
* In any case it holds pixel of a certain depth and format and can be drawn into.
*/
class PixelBuffer {

public:
    /**
    * Creates a PixelBuffer from of a given texture.
    */
    PixelBuffer(Texture* texture);
    void clear(glm::ivec4 color = {0, 0, 0, 255});

    void drawPoint(glm::vec2 position, glm::vec4 color);
    void drawLine(glm::ivec2 p1, glm::ivec2 p2, glm::vec4 color);
    void drawRect(glm::ivec2 topLeft, glm::ivec2 bottomRight, glm::vec4 color);


private:
    glm::vec2 dimension;
    PixelFormat format;
    std::span<uint8_t> pixels;

    void drawXDominant(glm::ivec2 p1, glm::ivec2 p2, glm::ivec4 color);
    void drawYDominant(glm::ivec2 p1, glm::ivec2 p2, glm::ivec4 color);

};


#endif //RASTERGRAPHICS_H
