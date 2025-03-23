//
// Created by mgrus on 22.03.2025.
//

#include "MeshImporter.h"
#include <graphics.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <engine/io/skeleton_import.h>

#include "ozz/animation/runtime/skeleton.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"


Animation* AssimpMeshImporter::aiAnimToAnimation(aiAnimation* aiAnim) {
    //printf("Animation: %s (%f) ticksPerSecond: %f\n", aiAnim->mName.C_Str(), aiAnim->mDuration, aiAnim->mTicksPerSecond);
    auto animation = new Animation();
    animation->name = aiAnim->mName.C_Str();
    animation->duration = aiAnim->mDuration / aiAnim->mTicksPerSecond;
    animation->ticksPerSecond = aiAnim->mTicksPerSecond;

    for (int c = 0; c < aiAnim->mNumChannels; c++) {
        auto channel = aiAnim->mChannels[c];
        auto jointName = channel->mNodeName.C_Str();

        for (int rk = 0; rk < channel->mNumRotationKeys; rk++) {
            auto sample = new AnimationSample();
            sample->type = SampleType::rotation;
            auto rotKey = channel->mRotationKeys[rk];
            sample->time = rotKey.mTime / aiAnim->mTicksPerSecond;
            sample->jointName = channel->mNodeName.C_Str();

            auto rot = assimpQuatToGLM(rotKey.mValue);
            sample->rotation = rot;
            animation->storeSample(sample, jointName);

        }

        for (int pk = 0; pk < channel->mNumPositionKeys; pk++) {
            auto sample = new AnimationSample();
            sample->type = SampleType::translation;
            sample->jointName = channel->mNodeName.C_Str();
            auto posKey = channel->mPositionKeys[pk];
            sample->time = posKey.mTime / aiAnim->mTicksPerSecond;
            glm::vec3 pos =  {posKey.mValue.x, posKey.mValue.y, posKey.mValue.z};
            sample->translation= pos;
            animation->storeSample(sample, jointName);
        }

    }

    return animation;
}

// Function to convert aiQuaternion to glm::quat
glm::quat AssimpMeshImporter::assimpQuatToGLM(const aiQuaternion& aiQuat) {
    // GLM's constructor expects (w, x, y, z)
    return glm::quat(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
}

Mesh * AssimpMeshImporter::importMesh(const std::string &filePath, const std::string& skeletonBaseFolder) {
    Assimp::Importer importer;
    auto scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_CalcTangentSpace);
    if (!scene) {
        throw new std::runtime_error("error during assimp scene load. ");
    }

    auto assimpAnimations = importAnimationsInternal(scene);
    auto ozzAnimations = importOzzAnimations();

    if (scene->mNumMeshes == 0) {
        return nullptr;
    }

    auto mesh = scene->mMeshes[0];
    std::vector<glm::vec3> posMasterList;
    std::vector<glm::vec3> posIndexSortedMasterList;
    std::vector<glm::vec2> uvMasterList;
    std::vector<glm::vec3> tangentMasterList;
    std::vector<glm::vec3> normalMasterList;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

        posMasterList.push_back({mesh->mVertices[i].x,
                                 mesh->mVertices[i].y,
                                 mesh->mVertices[i].z});

        if (mesh->mTangents) {
            tangentMasterList.push_back({
                mesh->mTangents[i].x,
                mesh->mTangents[i].y,
                mesh->mTangents[i].z,
            });
        }

        if (mesh->HasNormals()) {
            normalMasterList.push_back({mesh->mNormals[i].x,
                                        mesh->mNormals[i].y,
                                        mesh->mNormals[i].z});
        }

        if (mesh->mTextureCoords[0]) {
            uvMasterList.push_back({mesh->mTextureCoords[0][i].x,
                                    mesh->mTextureCoords[0][i].y});
        } else {
            uvMasterList.push_back({0.0f,
                                    0.0f});
        }
    }

    struct BoneWeight {
        std::string name = "";
        float weight = -1;
        int boneIndex = -1;
    };

    // Is this a skeletal mesh?
    std::vector<glm::ivec4> boneIndexMasterList;
    std::vector<glm::vec4> boneWeightMasterList;
    std::shared_ptr<ozz::animation::Skeleton> ozzSkeleton;
    Skeleton* skeleton = nullptr;
    if (mesh->HasBones()) {

#define IMPORT_SKELETON_OZZ_
#ifdef IMPORT_SKELETON_OZZ
        if (skeletonBaseFolder.empty()) {
            throw new std::runtime_error("error during assimp skeleton load, no base folder specified.");
        }
        OzzSkeletonImporter().importSkeleton(skeletonBaseFolder + "/skeleton.ozz", ozzSkeleton);
        if (ozzSkeleton) {
            for (auto bn : ozzSkeleton->joint_names()) {
                printf("joint name: %s\n", bn);
            }


        }

#endif

#define IMPORT_SKELETON_ASSIMP
#ifdef IMPORT_SKELETON_ASSIMP



            std::map<int, std::vector<BoneWeight*>*> weightMap;
            printf("bonedata: \n -------------------\n");

            skeleton = new Skeleton();

            for (int i = 0; i<mesh->mNumBones; i++) {
                auto bone = mesh->mBones[i];
                auto joint = new Joint();
                joint->name = bone->mName.C_Str();

                auto boneNode = FindNodeByName(scene->mRootNode, bone->mName.C_Str());

                // Does this bone have a parent or is it root?
                if (boneNode->mParent) {

                    // Do we know this parent already?
                    // i.e. we have already processed it and made a joint object for it.
                    // If yes, we just assign this joint as the parent to our newly created joint.
                    // If not, we create a new joint for this parent on the fly here.
                    // In fact, according to the ordering, the parent must always exist.
                    // We try to do nothing at all for now in this case.
                    if (auto parentJoint = findJointByName(boneNode->mParent->mName.C_Str(), skeleton->joints)) {
                        joint->setParent(parentJoint);
                    }
                }

                joint->inverseBindMatrix = convertAiMatrixToGlm(bone->mOffsetMatrix);
                joint->bindPoseLocalTransform = convertAiMatrixToGlm(boneNode->mTransformation);
                joint->bindPoseGlobalTransform = calculateBindPoseWorldTransform(joint, joint->bindPoseLocalTransform);
                joint->bindPoseFinalTransform = joint->bindPoseGlobalTransform * joint->inverseBindMatrix ;

                // One joint can influence many vertices, but any vertex can only be influenced by
                // maximum 4 joints.
                // Here we store every weight in a map which holds a list of weights per vertex.
                // The sum of all weights per vertex must be 1, not less, not more.
                for (int w = 0; w < bone->mNumWeights; w++) {
                    auto weight = bone->mWeights[w];
                    auto boneWeights = weightMap[weight.mVertexId];
                    if (!boneWeights) {
                        boneWeights = new std::vector<BoneWeight*>;
                        weightMap[weight.mVertexId] = boneWeights;

                    }
                    boneWeights->push_back(new BoneWeight{bone->mName.C_Str(), weight.mWeight, i});
                }
                skeleton->joints.push_back(joint);
            }

#ifdef DEBUG_PRINT_SKELETON
            // Print out all weights per mesh
            for (int i=0; i< mesh->mNumVertices; i++) {
                printf("vertex%d: %f/%f/%f\n", i, mesh->mVertices[i].x, mesh->mVertices[i].y,mesh->mVertices[i].z);
                float sum = 0;
                if (weightMap[i]) {
                    for (auto boneWeight : *weightMap[i]) {
                        if (boneWeight) {
                            printf("\t %s: %f\n", boneWeight->name.c_str(), boneWeight->weight);
                            sum+= boneWeight->weight;
                        }
                    }
                }

                printf("sum: %f\n\n", sum);

            }
#endif

            // Store all (at most 4) weights per vertex:
            for (int i=0; i< mesh->mNumVertices; i++) {
                float sum = 0;
                float count = 0;
                glm::ivec4 indices = {-1, -1, -1, -1};
                glm::vec4 weights = {0, 0, 0, 0};
                if (weightMap[i]) {
                    for (auto boneWeight : *weightMap[i]) {
                        sum+= boneWeight->weight;
                        count++;
                        if (count == 4) {
                            break;
                        }
                        if (count == 1) {
                            indices.x = boneWeight->boneIndex;
                            weights.x = boneWeight->weight;
                            continue;
                        }
                        if (count == 2) {
                            indices.y = boneWeight->boneIndex;
                            weights.y = boneWeight->weight;
                            continue;
                        }
                        if (count == 3) {
                            indices.z = boneWeight->boneIndex;
                            weights.z = boneWeight->weight;
                            continue;
                        }
                        if (count == 4) {
                            indices.w = boneWeight->boneIndex;
                            weights.w = boneWeight->weight;

                        }
                    }
                }
                boneIndexMasterList.push_back(indices);
                boneWeightMasterList.push_back(weights);
            }


#endif
    }

    auto posFlat = flattenV3(posMasterList);
    auto normFlat = flattenV3(normalMasterList);
    auto uvsFlat = flattenV2(uvMasterList);
    auto tangentsFlat = flattenV3(tangentMasterList);
    auto boneIndicesFlat = flattenIV4(boneIndexMasterList);
    auto boneWeightsFlat = flattenV4(boneWeightMasterList);

    std::vector<uint32_t> indicesFlat;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indicesFlat.push_back(face.mIndices[j]);
            posIndexSortedMasterList.push_back(posMasterList[face.mIndices[j]]);
        }
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint vboPos;
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, posFlat.size() * 4, posFlat.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);


    GLuint vbUvs;
    glGenBuffers(1, &vbUvs);
    glBindBuffer(GL_ARRAY_BUFFER, vbUvs);
    glBufferData(GL_ARRAY_BUFFER, uvsFlat.size() * 4, uvsFlat.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    GLuint vboNormals;
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normFlat.size() * 4, normFlat.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    GLuint vboTangents;
    glGenBuffers(1, &vboTangents);
    glBindBuffer(GL_ARRAY_BUFFER, vboTangents);
    glBufferData(GL_ARRAY_BUFFER, tangentsFlat.size() * 4, tangentsFlat.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(12, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(12);

    // Instance attribute for 2d offsets:
    uint32_t instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) , nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(3, 1);


    // Instance attribute for vec4 colors.
    uint32_t instanceColorVBO;
    glGenBuffers(1, &instanceColorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceColorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) , nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(10, 1);

    uint32_t instanceTintVBO;
    glGenBuffers(1, &instanceTintVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceTintVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) , nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(11);
    glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(11, 1);

    // Set up the attributes for the mat4 (4 vec4s) for more complex and complete transformation matrix
    // instancing:
    uint32_t matrixInstanceVBO;
    glGenBuffers(1, &matrixInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, matrixInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);

    int attribLocation = 6;
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(attribLocation + i);
        glVertexAttribPointer(attribLocation + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(attribLocation + i, 1);
    }

    // Attributes for skinned shader
    GLuint boneIndexBuffer;
    glGenBuffers(1, &boneIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, boneIndexBuffer);
    glBufferData(GL_ARRAY_BUFFER, boneIndicesFlat.size() * 4, boneIndicesFlat.data(), GL_STATIC_DRAW);
    glVertexAttribIPointer(4, 4, GL_INT, 0, 0);
    glEnableVertexAttribArray(4);


    GLuint boneWeightBuffer;
    glGenBuffers(1, &boneWeightBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, boneWeightBuffer);
    glBufferData(GL_ARRAY_BUFFER, boneWeightsFlat.size() * 4, boneWeightsFlat.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0,0);
    glEnableVertexAttribArray(5);

    GL_ERROR_EXIT(800)

    GLuint vboIndices;
    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesFlat.size() * 4, indicesFlat.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    auto mymesh = new Mesh();
    mymesh->vao = vao;
    mymesh->positionVBO = vboPos;
    mymesh->indicesVBO = vboIndices;
    mymesh->uvsVBO = vbUvs;
    mymesh->normalsVBO = vboNormals;
    mymesh->numberOfIndices = indicesFlat.size();
    mymesh->instanceOffsetVBO = instanceVBO;
    mymesh->instanceMatrixVBO = matrixInstanceVBO;
    mymesh->instanceColorVBO = instanceColorVBO;
    mymesh->instanceTintVBO = instanceTintVBO;
    mymesh->indexDataType = GL_UNSIGNED_INT;
    mymesh->positions = posMasterList;
    mymesh->uvs = uvMasterList;
    mymesh->positionsSortedByIndex = posIndexSortedMasterList;
    mymesh->indices = indicesFlat;
    mymesh->normals = normalMasterList;
    mymesh->tangents = tangentMasterList;
    mymesh->boneIndices = boneIndexMasterList;
    mymesh->boneWeights = boneWeightMasterList;
    mymesh->fileName = std::filesystem::path(filePath).filename().string();
    //mymesh->skeleton = skeleton.get();
    mymesh->ozzSkeleton = ozzSkeleton;
    mymesh->animations = assimpAnimations;
    mymesh->centroids = mymesh->calculateCentroids();

    return mymesh;

}


std::vector<Animation*> AssimpMeshImporter::importAnimations(const std::string &filePath) {
    Assimp::Importer importer;
    auto scene = importer.ReadFile(filePath,
                                   aiProcess_Triangulate |
                                   aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights);
    if (!scene) {
        printf("error during assimp scene load. ");
        exit(1);
    }

    return importAnimationsInternal(scene);
}

std::vector<std::shared_ptr<ozz::animation::Animation>> AssimpMeshImporter::importOzzAnimations(const std::string& skeletonBaseFolder) {

        if (skeletonBaseFolder.empty()) {
            return {};
        }

        std::vector<std::shared_ptr<ozz::animation::Animation>> animations;
        namespace fs = std::filesystem;
        auto directoryIterator = fs::directory_iterator(skeletonBaseFolder);
        for (const auto & entry : directoryIterator) {
            auto extension = entry.path().extension().string();
            auto entryName = entry.path().filename().stem().string();
            if (extension == ".ozz" && entryName != "skeleton.ozz" ) {
                std::string animationFileName = skeletonBaseFolder + "/" + animationFileName;
                std::cerr << "Loading animation archive: " << animationFileName << "."
                                << std::endl;
                ozz::io::File file(animationFileName.c_str(), "rb");
                if (!file.opened()) {
                    std::cerr << "Failed to open animation file " << animationFileName << "."
                                    << std::endl;
                    return {};
                }
                ozz::io::IArchive archive(&file);
                if (!archive.TestTag<ozz::animation::Animation>()) {
                    std::cerr << "Failed to load animation instance from file "
                                    << animationFileName << "." << std::endl;
                    return {};
                }

                auto animation = std::make_shared<ozz::animation::Animation>();
                auto animationRaw = new ozz::animation::Animation();
                archive >> *animationRaw;
                animation.reset(animationRaw);
                animations.push_back(animation);

            }
        }


        return animations;
}

std::vector<Animation*> AssimpMeshImporter::importAnimationsInternal(const aiScene *scene) {
    // Import animations
    std::vector<Animation*> animations;
    for (int i = 0; i < scene->mNumAnimations; i++ ) {
        auto aiAnim = scene->mAnimations[i];
        animations.push_back(aiAnimToAnimation(aiAnim));
    }

    return animations;
}