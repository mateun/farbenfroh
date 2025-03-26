//
// Created by mgrus on 25.03.2025.
//

#ifndef SPRITEBATCH_H
#define SPRITEBATCH_H

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <engine/graphics/TextureTile.h>

class MeshDrawData;

namespace gru {
    /***
     * This class allows for efficient rendering of many sprites in one go.
     * The idea is that we shall not issue a draw call per sprite.
     * Instead we collect all sprites to be rendered here.
     * Conveniently, the user can give us the sprites in pixel coordinates, which is most natural
     * for the game developer.
     * Internally we transform these coordinates into clipspace coordinates and store them in a list.
     * When the user calls "render", we make one update of the VBO containing every sprite.
     *
     */
    class SpriteBatch {

    public:

        SpriteBatch(int numberOfSpritesReservered);

        // Add a "sprite" by giving its position and size in screen space, i.e.
        // in pixels.
        // For example, if the screen is 640 x 480 and we give a sprite at position
        // 320/240, it will end up in clip space coordinates 0,0.
        // ClipSpace is -1 to 1 in all dimensions.
        // The formula is
        // clipSpace is: 2 * (pos/screenDimension) -1.
        // 2 * (320/640) -1 = 2 * 0.5 - 1 = 1 - 1 = 0
        // We use as a a depth indication
        int addSprite(glm::vec2 posInScreenSpace, glm::vec2 sizeInScreenSpace, TextureTile* textureTile, float zLayer = 0);

        // Render the batch, which means:
        // - update the VBO with all clipspace coordinates.
        // - (TODO) update the respective texture buffers, for now we have only one texture.
        void render(const MeshDrawData& mdd);

        void clear();

        void updateSprite(int index, glm::vec2 screenPos, glm::vec2 sizeInScreenSpace);

    private:
        // This is used to store the clips space position data which
        // is used for glBufferData at rendering time.
        // During the frame, data is added via addSprite in screen space,
        // then these coordinates are transformed to clip space, where they are stored in this list.
        std::vector<float> _clipSpacePositions;
        std::vector<float> _uvs;
        std::vector<uint16_t> _indices;

        GLuint _vbo;
        GLuint _vao;
        GLuint _ibo;
        GLuint _uvbo;

        uint16_t _spriteNumber = 0;
    };
}

#endif //SPRITEBATCH_H
