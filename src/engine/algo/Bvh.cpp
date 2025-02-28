//
// Created by mgrus on 14.06.2024.
//

#include "Bvh.h"
#include "../../graphics.h"

namespace gru {

    Bvh::Bvh(std::vector<Centroid*> centroids, glm::vec3 location, glm::vec3 scale) : location(location), scale(scale), centroids(centroids) {


        // No further splitting
        if (centroids.empty()) {
            return;
        }

        // Calculate the AABB of the passed in centroids.
        for (auto centroid : centroids) {
                if (centroid->a.x < outmostMin.x) {
                    outmostMin.x = centroid->a.x;
                }
                if (centroid->b.x < outmostMin.x) {
                    outmostMin.x = centroid->b.x;
                }
                if (centroid->c.x < outmostMin.x) {
                    outmostMin.x = centroid->c.x;
                }

                if (centroid->a.y < outmostMin.y) {
                    outmostMin.y = centroid->a.y;
                }
                if (centroid->b.y < outmostMin.y) {
                    outmostMin.y = centroid->b.y;
                }
                if (centroid->c.y < outmostMin.y) {
                    outmostMin.y = centroid->c.y;
                }
                if (centroid->a.z < outmostMin.z) {
                    outmostMin.z = centroid->a.z;
                }
                if (centroid->b.z < outmostMin.z) {
                    outmostMin.z = centroid->b.z;
                }
                if (centroid->c.z < outmostMin.z) {
                    outmostMin.z = centroid->c.z;
                }

                if (centroid->a.x > outmostMax.x) {
                    outmostMax.x = centroid->a.x;
                }
                if (centroid->b.x > outmostMax.x) {
                    outmostMax.x = centroid->b.x;
                }
                if (centroid->c.x > outmostMax.x) {
                    outmostMax.x = centroid->c.x;
                }
                if (centroid->a.y > outmostMax.y) {
                    outmostMax.y = centroid->a.y;
                }
                if (centroid->b.y > outmostMax.y) {
                    outmostMax.y = centroid->b.y;
                }
                if (centroid->c.y > outmostMax.y) {
                    outmostMax.y = centroid->c.y;
                }
                if (centroid->a.z > outmostMax.z) {
                    outmostMax.z = centroid->a.z;
                }
                if (centroid->b.z > outmostMax.z) {
                    outmostMax.z = centroid->b.z;
                }
                if (centroid->c.z > outmostMax.z) {
                    outmostMax.z = centroid->c.z;
                }
            }

        printf("aab defined: %f %f %f", outmostMax.x, outmostMax.y, outmostMax.z);

        // Next we split - into two groups, for a binary tree.
        // To find the splitting axis, we decide for the biggest dimension based on our AABB.
        // Then we find the median centroid on this axis of all the triangles, and this is then where the split is done.
        float extentX = abs(outmostMax.x - outmostMin.x);
        float extentY = abs(outmostMax.y - outmostMin.y);
        float extentZ = abs(outmostMax.z - outmostMin.z);
        float extents[3] = {extentX, extentY, extentZ};
        float maxExtent = 0;
        int index = -1;
        for (int i = 0; i < 3; i++) {
            if (abs(extents[i]) > maxExtent) {
                maxExtent = abs(extents[i]);
                index = i;
            }
        }


        if (index == 0) {
            // split by x
            std::ranges::sort(centroids.begin(), centroids.end(), [](const Centroid* a, const Centroid* b) {
                return a->value.x < b->value.x; // Sorts in ascending order by the y component.
            });
            auto medianElement = centroids[centroids.size() / 2];
            printf("median element x : %f\n", medianElement->value.x);

            // Assign our centroids to the respective child BVHs, so we have our recursion:
            std::vector<Centroid*> lefts;
            std::vector<Centroid*> rights;
            for (int i = 0; i < centroids.size()/2; i++) {
                lefts.push_back(centroids[i]);
            }
            for (int i = centroids.size()/2; i < centroids.size(); i++) {
                rights.push_back(centroids[i]);
            }
            if (lefts.size() > 5) {
                left = new Bvh(lefts, location, scale);
            }

            if (rights.size() > 5) {
                right = new Bvh(rights, location  + glm::vec3(medianElement->value.x, 0, 0), scale);
            }

        }
        else if (index == 1) {
            // split by y

            std::ranges::sort(centroids.begin(), centroids.end(), [](const Centroid* a, const Centroid* b) {
                return a->value.y < b->value.y; // Sorts in ascending order by the y component.
            });
            auto medianElement = centroids[centroids.size() / 2];
            printf("median element y : %f\n", medianElement->value.y);

            // Assign our centroids to the respective child BVHs, so we have our recursion:
            std::vector<Centroid*> lefts;
            std::vector<Centroid*> rights;
            for (int i = 0; i < centroids.size()/2; i++) {
                lefts.push_back(centroids[i]);
            }
            for (int i = centroids.size()/2; i < centroids.size(); i++) {
                rights.push_back(centroids[i]);
            }
            if (lefts.size() > 5) {
                left = new Bvh(lefts, location, scale);
            }
            if (rights.size() > 5) {
                right = new Bvh(rights, location + glm::vec3(0, medianElement->value.y, 0), scale);
            }

        }
        else if (index == 2) {
            // split by z
            std::ranges::sort(centroids.begin(), centroids.end(), [](const Centroid* a, const Centroid* b) {
                return a->value.z < b->value.z; // Sorts in ascending order by the y component.
            });
            auto medianElement = centroids[centroids.size() / 2];
            printf("median element z : %f\n", medianElement->value.z);

            // Assign our centroids to the respective child BVHs, so we have our recursion:
            std::vector<Centroid*> lefts;
            std::vector<Centroid*> rights;
            for (int i = 0; i < centroids.size()/2; i++) {
                lefts.push_back(centroids[i]);
            }
            for (int i = centroids.size()/2; i < centroids.size(); i++) {
                rights.push_back(centroids[i]);
            }
            if (lefts.size() > 5) {
                left = new Bvh(lefts, location , scale);
            }
            if (rights.size() > 5) {
                right = new Bvh(rights, location + glm::vec3(0, 0, medianElement->value.z), scale);
            }
        }

    }

    Mesh * Bvh::getCubeMesh() {
        if (!cubeMesh) {
            cubeMesh = MeshImporter().importMesh("../assets/unit_cube.glb");
        }
        return cubeMesh;

    }

    Shader* Bvh::getShader() {
        if (!unitCubeShader) {
            unitCubeShader = new Shader();
            unitCubeShader->initFromFiles("../src/engine/algo/shaders/wireframe.vert", "../src/engine/algo/shaders/wireframe.frag");
        }
        return unitCubeShader;
    }

    void Bvh::render(Camera *camera, int level, const std::string& kind) {

        if (level > 4) {
            return;
        }

        MeshDrawData mdd;
        mdd.camera = camera;
        mdd.mesh = getCubeMesh();
        mdd.shader = getShader();

        // We need to calculate an offset to add to our AABB.
        // The model might not be symmetrical, but our unit cube is.
        // So we need to offset it by the actual asymmetry on all axes.
        glm::vec3 absDiff = (glm::abs(outmostMax) - glm::abs(outmostMin)) / 2.0f;
        mdd.location = location + glm::vec3(absDiff.x, absDiff.y, absDiff.z);
        //mdd.location = location + glm::vec3(0, 1, 0);

        mdd.scale = (outmostMax - outmostMin) * scale * (1 + level * 0.05f);
        if (kind == "top") {
            mdd.color = glm::vec4(1, 0, 1, 1);
            wireframeOn(4);
        } else if (kind == "left") {
            mdd.color = glm::vec4((float)level / 5.0, 0, 0, 1);
            wireframeOn(3);
        } else if (kind == "right") {
            wireframeOn(2);
            mdd.color = glm::vec4(0, (float)level/5.0, 0, 1);
        }


        drawMesh(mdd);
        wireframeOff();
        if (left) {
            left->render(camera, level+1, "left");
        }
        if (right) {
            right->render(camera, level+1, "right");
        }

    }
} // gru