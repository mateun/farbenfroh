//
// Created by mgrus on 22.03.2025.
//

#ifndef MESHIMPORTER_H
#define MESHIMPORTER_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <ozz/animation/runtime/animation.h>
#include <engine/animation/Animation.h>
#include <assimp/quaternion.h>
#include <assimp/matrix4x4.h>

class Mesh;
struct aiScene;
struct aiNode;
struct aiAnimation;


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

    static glm::quat assimpQuatToGLM(const aiQuaternion& aiQuat);

    static const aiNode *FindNodeByName(const aiNode *node, const std::string &name);

    static glm::mat4 convertAiMatrixToGlm(const aiMatrix4x4& from);

    static glm::mat4 calculateBindPoseWorldTransform(Joint* j, glm::mat4 currentTransform);


    glm::mat4 calculateWorldTransformForFrame(Joint* j, glm::mat4 currentTransform, int frame);

    Animation* aiAnimToAnimation(aiAnimation* aiAnim);

    static std::vector<std::shared_ptr<ozz::animation::Animation>> importOzzAnimations(const std::string& skeletonBaseFolder = "");

};


#endif //MESHIMPORTER_H
