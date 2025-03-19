//
// Created by mgrus on 19.03.2025.
//

#ifndef SCENENODE_H
#define SCENENODE_H

#include <glm/glm.hpp>
#include <string>
#include "graphics.h"

class NodeComponent;
class MeshDrawData;
namespace gru {
class ParticleSystem;
}
class Camera;
class Light;



class SceneNode {
    friend class Scene;

public:
    SceneNode(const std::string& nodeId = "undefined");
    ~SceneNode();

    void initAsMeshNode(const MeshDrawData& meshData);
    void initAsParticleSystemNode(gru::ParticleSystem* particleSystem);
    void initAsCameraNode(Camera* camera);
    void initAsLightNode(Light* light);
    void initAsTextNode();  // TODO

    std::string getId();

    void yaw(float degrees);

    glm::vec3 getForwardVector();
    glm::vec3 getRightVector();
    glm::vec3 getVelocity();

    bool isActive();

    void setLocation(glm::vec3 vec);
    void setScale(glm::vec3 _scale);
    void setRotation(glm::vec3 rotationInEulers, AngleUnit angleUnit = AngleUnit::DEGREES);
    void setOrientation(glm::quat orientation);

    glm::vec3 getHierarchicalWorldLocation(glm::vec3 localLocation);
    glm::quat getWorldOrientation();

    void collectParentNodes(std::vector<SceneNode *>& parents);

    glm::vec3 getLocation();
    glm::vec3 getScale();

    Camera * getCamera();

    void updateBoneMatrices(std::vector<glm::mat4> get_bone_matrices);

    const std::vector<glm::mat4>& boneMatrices();

    void disable();
    void enable();

    void setParent(SceneNode * scene_node);

    void addChild(SceneNode *child);

    MeshDrawData getMeshData();

    void setExtraData(void* data);
    void* getExtraData();

    void udpate();
    void addComponent(NodeComponent * component);

    glm::quat getOrientation();

private:
    std::vector<NodeComponent*> components;

    // These are type specific fields and can be null
    std::string id;
    Mesh* mesh = nullptr;
    gru::ParticleSystem* particleSystem = nullptr;
    Texture* texture = nullptr;
    Texture* normalMap = nullptr;
    Shader* shader = nullptr;
    Camera* camera = nullptr;
    Light* light = nullptr;
    MeshDrawData meshData;

    glm::vec3 _location = glm::vec3(0);
    glm::vec3 _velocity = glm::vec3(0);
    glm::vec3 _scale = glm::vec3(1.0f);
    glm::vec3 _rotationInDeg = glm::vec3(0);
    glm::quat orientation = glm::identity<glm::quat>();
    glm::vec3 forward= {0, 0, -1};   // This is normally how we face when coming from Blender
    glm::vec3 right = {1, 0, 0}; // Based on the incoming Blender default orientation
    glm::vec4 foregroundColor = {1, 0,1, 1};
    float uvScale = 1;
    glm::vec2 uvScale2 = {1, 1};
    glm::vec2 uvPan = {0, 0};
    bool skinnedMesh = false;
    std::vector<glm::mat4> _boneMatrices;
    SceneNodeType _type;
    bool _active = true;
    SceneNode * parent = nullptr;
    std::vector<SceneNode*> children;

    void* extraData = nullptr;

};




#endif //SCENENODE_H
