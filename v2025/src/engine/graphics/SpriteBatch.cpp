//
// Created by mgrus on 25.03.2025.
//

#define NOMINMAX
#include "SpriteBatch.h"
#include <algorithm>
#include "Application.h"
#include "MeshDrawData.h"
#include "Shader.h"


/**
 * Create our VAO and VBO in sufficient size to hold all potential sprites.
 * We just reserve memory here, but we do not yet fill it.
 * This happens during the render operation.
 */
gru::SpriteBatch::SpriteBatch(int numberOfSpritesToBeReserved) {

    _clipSpacePositions.reserve(12 * numberOfSpritesToBeReserved);
    _uvs.reserve(8 * numberOfSpritesToBeReserved);
    _indices.reserve(6 * numberOfSpritesToBeReserved);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    //    float positions[] = {
    //            -0.5, 0.5, 0,   // top left
    //            -0.5, -0.5, 0,  // bottom left
    //            0.5, -0.5, 0,   // bottom right
    //            0.5, 0.5, 0                         // top right
    //    };

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    long int posSize = 4 * 12 * numberOfSpritesToBeReserved * 3;
    glBufferData(GL_ARRAY_BUFFER, posSize, nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // UVS
    {
        //        float uvs[] = {
        //                0, 1,
        //                0, 0,
        //                1, 0,
        //                1, 1,
        //
        //        };
        glGenBuffers(1, &_uvbo);
        glBindBuffer(GL_ARRAY_BUFFER, _uvbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8 *  numberOfSpritesToBeReserved * 3, nullptr, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
    }

    GLuint indices[] = {
        0, 1, 2,
        0, 2, 3
};

    glGenBuffers(1, &_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort ) * 6 * numberOfSpritesToBeReserved * 3, nullptr, GL_STATIC_DRAW);

    auto err = glGetError();
    if (err != 0) {
        printf("gl Error");
    }
    glBindVertexArray(0);

}



int gru::SpriteBatch::addSprite(glm::vec2 posInScreenSpace, glm::vec2 sizeInScreenSpace, gru::TextureTile* textureTile, float zLayer) {

    auto window_width = getApplication()->scaled_width();
    auto window_height = getApplication()->scaled_height();
    // pos to clip space
    double clipSpaceX = (2.0 * (posInScreenSpace.x/ window_width)) - 1.0;
    double clipSpaceY = (2.0 * (posInScreenSpace.y/ window_height)) - 1.0;
    double clipSpaceWidth = (2.0 * (sizeInScreenSpace.x/ window_width));
    double clipSpaceHeight = (2.0 * (sizeInScreenSpace.y/ window_height));
    double leftTopX = clipSpaceX - (clipSpaceWidth/2);
    double leftTopY = clipSpaceY + (clipSpaceHeight/2);
    double rightTopX = clipSpaceX + (clipSpaceWidth/2);
    double rightTopY = clipSpaceY + (clipSpaceHeight/2);
    double leftBottomX = clipSpaceX - (clipSpaceWidth/2);
    double leftBottomY = clipSpaceY - (clipSpaceHeight/2);
    double rightBottomX = clipSpaceX + (clipSpaceWidth/2);
    double rightBottomY = clipSpaceY - (clipSpaceHeight/2);

    #define PB(value) _clipSpacePositions.push_back(value);
    PB(leftTopX) PB(leftTopY) PB(zLayer)
    PB(rightTopX) PB(rightTopY) PB(zLayer)
    PB(leftBottomX) PB(leftBottomY) PB(zLayer)
    PB(rightBottomX) PB(rightBottomY) PB(zLayer)

    // Also update the indices.
    // This depends on the current sprite, so
    // we multiply by the sprite index.
    _indices.push_back(4 * _spriteNumber + 0);
    _indices.push_back(4 * _spriteNumber + 2);
    _indices.push_back(4 * _spriteNumber + 1);
    _indices.push_back(4 * _spriteNumber + 1);
    _indices.push_back(4 * _spriteNumber + 2);
    _indices.push_back(4 * _spriteNumber + 3);


    // And update the uvs
    // Based on the texture tile.
    if (textureTile) {
        auto uvs = textureTile->getTextureCoords();
        _uvs.push_back(uvs.left);
        _uvs.push_back(uvs.top);
        _uvs.push_back(uvs.right);
        _uvs.push_back(uvs.top);
        _uvs.push_back(uvs.left);
        _uvs.push_back(uvs.bottom);
        _uvs.push_back(uvs.right);
        _uvs.push_back(uvs.bottom);
    } else {
        _uvs.push_back(0);
        _uvs.push_back(1);
        _uvs.push_back(1);
        _uvs.push_back(1);
        _uvs.push_back(0);
        _uvs.push_back(0);
        _uvs.push_back(1);
        _uvs.push_back(0);
    }


    _spriteNumber += 1;
    if (_indices.size() > std::numeric_limits<uint16_t>::max()) {
        exit(100);
    }

    return _spriteNumber;

}

void gru::SpriteBatch::render(const MeshDrawData& mdd) {
    glBindVertexArray(_vao);
    mdd.shader->bind();
    auto err = glGetError();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mdd.texture->handle());

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,  _clipSpacePositions.size()* 4, _clipSpacePositions.data());
    auto err1 = glGetError();


    glBindBuffer(GL_ARRAY_BUFFER, _uvbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,  _uvs.size()*  4, _uvs.data());
    auto err2 = glGetError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,  _indices.size()* 2 , _indices.data());
    auto err3 = glGetError();

    glDrawElements(GL_TRIANGLES, _spriteNumber * 6, GL_UNSIGNED_SHORT, nullptr);
    auto err4 = glGetError();

    //err = glGetError();
    if (err != 0) {
        printf("glError %d\n", err);
    }

    // Maybe we don't need this
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);




}

void gru::SpriteBatch::clear() {
    // Reset everything
    _spriteNumber = 0;
    _clipSpacePositions.clear();
    _uvs.clear();
    _indices.clear();

}

void gru::SpriteBatch::updateSprite(int index, glm::vec2 posInScreenSpace, glm::vec2 sizeInScreenSpace) {
    double clipSpaceX = (2.0 * (posInScreenSpace.x/ getApplication()->scaled_width())) - 1.0;
    double clipSpaceY = (2.0 * (posInScreenSpace.y/ getApplication()->scaled_height())) - 1.0;
    double clipSpaceWidth = (2.0 * (sizeInScreenSpace.x/ getApplication()->scaled_width()));
    double clipSpaceHeight = (2.0 * (sizeInScreenSpace.y/ getApplication()->scaled_height()));
    double leftTopX = clipSpaceX - (clipSpaceWidth/2);
    double leftTopY = clipSpaceY + (clipSpaceHeight/2);
    double rightTopX = clipSpaceX + (clipSpaceWidth/2);
    double rightTopY = clipSpaceY + (clipSpaceHeight/2);
    double leftBottomX = clipSpaceX - (clipSpaceWidth/2);
    double leftBottomY = clipSpaceY - (clipSpaceHeight/2);
    double rightBottomX = clipSpaceX + (clipSpaceWidth/2);
    double rightBottomY = clipSpaceY - (clipSpaceHeight/2);

    int startIndex = index * 12;
    _clipSpacePositions[startIndex] = leftTopX;
    _clipSpacePositions[startIndex+1] = leftTopY;
    _clipSpacePositions[startIndex+2] = -0.5;
    _clipSpacePositions[startIndex+3] = rightTopX;
    _clipSpacePositions[startIndex+4] = rightTopY;
    _clipSpacePositions[startIndex+5] = -0.5;
    _clipSpacePositions[startIndex+6] = leftBottomX;
    _clipSpacePositions[startIndex+7] = leftBottomY;
    _clipSpacePositions[startIndex+8] = -0.5;
    _clipSpacePositions[startIndex+9] = rightBottomX;
    _clipSpacePositions[startIndex+10] = rightBottomY;
    _clipSpacePositions[startIndex+11] = -0.5;
}



