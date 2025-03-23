//
// Created by mgrus on 22.03.2025.
//

#ifndef MESHIMPORTER_H
#define MESHIMPORTER_H

#include <glm/glm.hpp>
#include <string>


class MeshImporter {
public:
    virtual ~MeshImporter() = default;

    virtual Mesh* importMesh(const std::string& filePath, const std::string& skeletonBaseFolder = "") = 0;

    virtual std::vector<Animation*> importAnimations(const std::string& filePath) = 0;


};

class AssimpMeshImporter : public MeshImporter {

public:

    /**
     * This uses the Assimp library to import the mesh and animations if available.
     * FBX notes:
     * For .fbx files out of Blender, first rotate the object by -90 degress around the x-axis.
     * Then apply all transforms.
     * Then export fbx.
     *
     * GLTF notes:
     * Disable "Sample optimization" and "Keep channel..." (or similar settings).
     * This will lead to exact non optimized frame information, making it easier to debug.
     *
    **/
    Mesh* importMesh(const std::string& filePath, const std::string& skeletonBaseFolder = "") override;

    // Uses Assimp to import animations only.
    std::vector<Animation *> importAnimations(const std::string &filePath) override;
private:
    std::vector<Animation*> importAnimationsInternal(const aiScene* scene);
    glm::quat assimpQuatToGLM(const aiQuaternion& aiQuat);
    Animation* aiAnimToAnimation(aiAnimation* aiAnim);
    std::vector<std::shared_ptr<ozz::animation::Animation>> importOzzAnimations(const std::string& skeletonBaseFolder = "");

};


#endif //MESHIMPORTER_H
