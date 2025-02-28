//
// Created by mgrus on 14.06.2024.
//

#ifndef SIMPLE_KING_BVH_H
#define SIMPLE_KING_BVH_H

#include "graphics.h"
class Centroid;
class Camera;
class Shader;
class Mesh;

namespace gru {

    enum class SplitAxis { X, Y, Z, NONE };




    struct TreeNode {
        TreeNode* left = nullptr;
        TreeNode* right = nullptr;

    };

    class Bvh {
    public:

        // Create a BVH from a mesh.
        // The meshes triangles are split into the different bounding volumes.
        Bvh(std::vector<Centroid*> centroids, glm::vec3 location, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f));


        // Visualize the BVH with transparent cubes.
        void render(Camera* camera, int level = 0, const std::string& kind = "top");

    private:

        // Lazy init the internal cube mesh
        Mesh * getCubeMesh();

        Shader *getShader();

        // World location of the mesh
        glm::vec3 location;

        // Scale applied to this instance
        glm::vec3 scale;

        // A cube mesh used for visualization
        Mesh* cubeMesh = nullptr;



        Shader * unitCubeShader = nullptr;

        glm::vec3 outmostMin =  {1000, 1000, 1000};
        glm::vec3 outmostMax =  {0, 0, 0};

        Bvh* left = nullptr;
        Bvh* right = nullptr;
        std::vector<Centroid *> centroids;
    };

} // gru

#endif //SIMPLE_KING_BVH_H
