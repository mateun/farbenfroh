//
// Created by mgrus on 26.03.2025.
//

#ifndef TEXTURETILE_H
#define TEXTURETILE_H

#include <glm/glm.hpp>


namespace gru {

    struct UVs {
        float left;
        float top;
        float right;
        float bottom;
    };

    // A tile in a tileMap in pixel coordinates.
    struct TextureTile {

      TextureTile(glm::vec2 tl, glm::vec2 br, glm::vec2 atl) {
          topLeft = tl;
          bottomRight = br;
          atlas = atl;
      }

      glm::vec2 topLeft;
      glm::vec2 bottomRight;
      glm::vec2 atlas;

      UVs getTextureCoords() {
          auto tileWidth = bottomRight.x - topLeft.x;
          auto tileHeight = bottomRight.y - topLeft.y;
          float uvWidth = tileWidth / atlas.x;
          float uvHeight = tileHeight / atlas.y;
          float uvLeft = topLeft.x / atlas.x;
          float uvRight = uvLeft + uvWidth;
          float uvTop = 1 -  (topLeft.y / atlas.y);
          float uvBottom = uvTop - uvHeight;

          return { uvLeft, uvTop, uvRight, uvBottom};
      }
    };
}



#endif //TEXTURETILE_H
