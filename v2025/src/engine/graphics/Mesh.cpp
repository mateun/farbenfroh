//
// Created by mgrus on 26.03.2025.
//


#include <engine\graphics\Mesh.h>
#include <engine\physics\Ray.h>
#include <engine/physics/Centroid.h>

/**
 * This function checks if the passed in ray collides with any of
 * the triangles of this mesh.
 *
 * @param color The color of the hitpoint of the ray. Only valid if there was
 *              a collision registered.
 * @return true if we have a collision, falls if not.
 */
bool Mesh::rayCollides(Ray ray, glm::vec4 &color) {
    // Build the triangles
    for (int i = 0; i < indices.size(); i += 3) {
        glm::vec3 v1 = positions[indices[i]];
        glm::vec3 v2 = positions[indices[i+1]];
        glm::vec3 v3 = positions[indices[i+2]];
        // Check ray against this triangle
    }
    return false;
}

Animation * Mesh::findAnimation(const std::string &name) {
    for (auto a: animations) {
        if (a->name == name) {
            return a;
        }
    }
    return nullptr;
}

std::vector<Centroid *> Mesh::calculateCentroids() {
    if (centroids.empty()) {
        for (int i = 0; i < indices.size(); i += 3) {
            auto a = positions[indices[i]];
            auto b = positions[indices[i+1]];
            auto c = positions[indices[i+2]];
            auto centroidValue = (a + b + c) / 3.0f;
            auto centroid = new Centroid{a, b, c, centroidValue};
            centroids.push_back(centroid);
        }
    }

    return centroids;
}
