//
// Created by mgrus on 14.06.2024.
//

#include "Bvh.h"

namespace gru {
    Bvh::Bvh(Mesh *mesh) {
        // First create an AABB to encompass all triangles:
        // leftmost coordinate:
        float minX = 10000;
        float minY = 10000;
        float minZ = 10000;
        float maxX = -10000;
        float maxY = -10000;
        float maxZ = -10000;

        for (auto pos : mesh->positions) {
            if (pos.x < minX) {
                minX = pos.x;
            }
            if (pos.y < minY) {
                minY = pos.y;
            }
            if (pos.z < minZ) {
                minZ = pos.z;
            }

            if (pos.x > maxX) {
                maxX = pos.x;
            }
            if (pos.y > maxY) {
                maxY = pos.y;
            }
            if (pos.z > maxZ) {
                maxZ = pos.z;
            }
        }

        printf("aab defined: %f %f %f", minX, minY, minZ);

    }
} // gru