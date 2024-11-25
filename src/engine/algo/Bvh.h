//
// Created by mgrus on 14.06.2024.
//

#ifndef SIMPLE_KING_BVH_H
#define SIMPLE_KING_BVH_H

#include "graphics.h"

namespace gru {

    struct TreeNode {
        TreeNode* left = nullptr;
        TreeNode* right = nullptr;

    };

    class Bvh {
    public:

        // Create a BVH from a mesh.
        // The meshes triangles are split into the different bounding volumes.
        Bvh(Mesh* mesh);


    };

} // gru

#endif //SIMPLE_KING_BVH_H
