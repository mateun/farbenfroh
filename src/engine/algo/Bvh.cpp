//
// Created by mgrus on 14.06.2024.
//

#include "Bvh.h"

#include <engine/io/MeshImporter.h>
#include <engine/graphics/Shader.h>
#include <engine/physics/Centroid.h>
#include <engine/graphics/MeshDrawData.h>

namespace gru {

    Bvh::Bvh(std::vector<Centroid*> centroids, glm::vec3 location, glm::vec3 locationOffset, glm::vec3 scale, const std::string& kind, SplitAxis splitAxis) : visualizationLocationOffset(locationOffset), location(location), scale(scale), centroids(centroids) {


        // No further splitting
        if (centroids.empty()) {
            return;
        }

        // Calculate the AABB of the passed in centroids.
        for (auto centroid : centroids) {
                // if (centroid->value.x < outmostMin.x) {
                //     outmostMin.x = centroid->value.x;
                // }

                if (centroid->a.x < outmostMin.x) {
                    outmostMin.x = centroid->a.x;
                }
                if (centroid->b.x < outmostMin.x) {
                    outmostMin.x = centroid->b.x;
                }
                if (centroid->c.x < outmostMin.x) {
                    outmostMin.x = centroid->c.x;
                }

            // if (centroid->value.y < outmostMin.y) {
            //     outmostMin.y = centroid->value.y;
            // }

                if (centroid->a.y < outmostMin.y) {
                    outmostMin.y = centroid->a.y;
                }
                if (centroid->b.y < outmostMin.y) {
                    outmostMin.y = centroid->b.y;
                }
                if (centroid->c.y < outmostMin.y) {
                    outmostMin.y = centroid->c.y;
                }

            // if (centroid->value.z < outmostMin.z) {
            //     outmostMin.z = centroid->value.z;
            // }
                if (centroid->a.z < outmostMin.z) {
                    outmostMin.z = centroid->a.z;
                }
                if (centroid->b.z < outmostMin.z) {
                    outmostMin.z = centroid->b.z;
                }
                if (centroid->c.z < outmostMin.z) {
                    outmostMin.z = centroid->c.z;
                }

            // if (centroid->value.x > outmostMax.x) {
            //     outmostMax.x = centroid->value.x;
            // }

                if (centroid->a.x > outmostMax.x) {
                    outmostMax.x = centroid->a.x;
                }
                if (centroid->b.x > outmostMax.x) {
                    outmostMax.x = centroid->b.x;
                }
                if (centroid->c.x > outmostMax.x) {
                    outmostMax.x = centroid->c.x;
                }

                // if (centroid->value.y > outmostMax.y) {
                //     outmostMax.y = centroid->value.y;
                // }

                if (centroid->a.y > outmostMax.y) {
                    outmostMax.y = centroid->a.y;
                }
                if (centroid->b.y > outmostMax.y) {
                    outmostMax.y = centroid->b.y;
                }
                if (centroid->c.y > outmostMax.y) {
                    outmostMax.y = centroid->c.y;
                }

            // if (centroid->value.z > outmostMax.z) {
            //     outmostMax.z = centroid->value.z;
            // }

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

        printf("AABB defined for BVH %s: (%f/%f/%f) - (%f %f %f). Offset: %f/%f/%f", kind.c_str(), outmostMin.x, outmostMin.y, outmostMin.z, outmostMax.x, outmostMax.y, outmostMax.z, locationOffset.x, locationOffset.y, locationOffset.z);

        // Calculate corrected visualization offset based on our split axis
        // already found AABB:

        if (splitAxis == SplitAxis::X) {
            this->visualizationLocationOffset.x = outmostMin.x;
        }

        if (splitAxis == SplitAxis::Y) {
            this->visualizationLocationOffset.y = outmostMin.y;
        }
        if (splitAxis == SplitAxis::Z) {
            this->visualizationLocationOffset.z = outmostMin.z;
        }

        printf("\t locationOffset corrected: %f/%f/%f: ", visualizationLocationOffset.x, visualizationLocationOffset.y, visualizationLocationOffset.z);

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

        // X Axis split
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
                left = new Bvh(lefts, location, {0,0,0}, scale, "left", SplitAxis::X);
            }

            if (rights.size() > 5) {
                right = new Bvh(rights, location, glm::vec3(medianElement->value.x, 0, 0), scale, "right", SplitAxis::X);
            }

        }
        // Y Axis Split
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
                left = new Bvh(lefts, location, {0,0,0}, scale, "left", SplitAxis::Y);
            }
            if (rights.size() > 5) {
                right = new Bvh(rights, location, glm::vec3(0, medianElement->value.y, 0), scale, "right", SplitAxis::Y);
            }

        }
        // Z Axis split
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
                left = new Bvh(lefts, location , {0,0,0}, scale, "left", SplitAxis::Z);
            }
            if (rights.size() > 5) {
                right = new Bvh(rights, location, glm::vec3(0, 0, medianElement->value.z), scale, "right", SplitAxis::Z);
            }
        }

    }

    Mesh * Bvh::getCubeMesh() {
        if (!cubeMesh) {
            cubeMesh = AssimpMeshImporter().importMesh("../assets/unit_cube.glb");
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

        if (level > 3) {
            return;
        }

        MeshDrawData mdd;
        mdd.camera = camera;
        mdd.mesh = getCubeMesh();
        mdd.shader = getShader();

        // We need to calculate an offset to add to our AABB.
        // The model might not be symmetrical, but our unit cube is.
        // So we need to offset it by the actual asymmetry on all axes.
        glm::vec3 absDiff = (glm::abs(outmostMax) - glm::abs(outmostMin));
        mdd.location = location + (absDiff/2.0f) + (visualizationLocationOffset);

        mdd.scale = (outmostMax - outmostMin) * scale * (1 - level * 0.01f);
        if (kind == "top") {
            mdd.color = glm::vec4(1, 0, 1, 1);
            wireframeOn(1);
        } else if (kind == "left") {
            mdd.color = glm::vec4(0.95, 0, 0, 1);
            wireframeOn(1);
        } else if (kind == "right") {
            wireframeOn(1);
            mdd.color = glm::vec4(0, 0.95, 0, 1);
        }

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        drawMesh(mdd);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        wireframeOff();
        if (left) {
            left->render(camera, level+1, "left");
        }
        if (right) {
            right->render(camera, level+1, "right");
        }
    }
} // gru