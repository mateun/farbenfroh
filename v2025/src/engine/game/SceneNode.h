//
// Created by mgrus on 19.03.2025.
//

#ifndef SCENENODE_H
#define SCENENODE_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "NodeComponent.h"
#include <engine/math/data_structures.h>
#include <engine/graphics/Camera.h>
#include <engine/graphics/MeshDrawData.h>

class NodeComponent;
struct MeshDrawData;
namespace gru {
class ParticleSystem;
}
class Camera;
class Light;



enum class SceneNodeType {
    Light,
    Camera,
    Text,
    Mesh,
    ParticleSystem,
};


class SceneNode : public std::enable_shared_from_this<SceneNode> {
    friend class Scene;

public:
    SceneNode(const std::string& nodeId, std::shared_ptr<NodeTransform> transform = DEFAULT_TRANSFORM);
    ~SceneNode();

    void initAsMeshNode(const MeshDrawData& meshData);
    void initAsParticleSystemNode(std::shared_ptr<gru::ParticleSystem> particleSystem);
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

    void collectParentNodes(std::vector<std::shared_ptr<SceneNode>>& parents);

    glm::vec3 getLocation();
    glm::vec3 getScale();

    Camera * getCamera() const;

    void updateBoneMatrices(std::vector<glm::mat4> get_bone_matrices);

    const std::vector<glm::mat4>& boneMatrices();

    void update();
    void disable();
    void enable();

    void setParent(const std::shared_ptr<SceneNode>& scene_node);
    void addChild(std::shared_ptr<SceneNode> child);

    MeshDrawData& getMeshData();

    void setExtraData(void* data);
    void* getExtraData();

    template <typename T>
    std::vector<std::shared_ptr<T> > getComponents() const;
    void addComponent(std::unique_ptr<NodeComponent> component);

    glm::quat getOrientation();

    std::shared_ptr<NodeTransform> transform();

    std::vector<std::shared_ptr<SceneNode>> findChildren(SceneNodeType scene_node);

private:
    std::vector<std::shared_ptr<NodeComponent>> _components;

    // These are type specific fields and can be null
    std::string id;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<gru::ParticleSystem> particleSystem = nullptr;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Texture> normalMap;
    std::shared_ptr<Shader> shader;
    Camera* camera = nullptr;
    Light* light = nullptr;
    MeshDrawData meshData;

    std::shared_ptr<NodeTransform> _transform;
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

    std::weak_ptr<SceneNode> parent;
    std::vector<std::shared_ptr<SceneNode>> children;

    void* extraData = nullptr;

};

template<typename T>
std::vector<std::shared_ptr<T>> SceneNode::getComponents() const {
    std::vector<std::shared_ptr<T> > components;
    for (const auto& comp : _components) {
        // Use dynamic_cast to check if the component is of type T
        if (auto specific = std::dynamic_pointer_cast<T>(comp)) {
            components.push_back(specific);
        }
    }
    return components;
}


#endif //SCENENODE_H
