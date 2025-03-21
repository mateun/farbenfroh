//
// Created by mgrus on 19.03.2025.
//

#ifndef NODECOMPONENT_H
#define NODECOMPONENT_H

enum class Space {
  WORLD,
  LOCAL,
};

#define DEFAULT_TRANSFORM std::make_shared<NodeTransform>()

class NodeTransform {

public:
  NodeTransform();
  NodeTransform(std::shared_ptr<NodeTransform> parent);
  glm::vec3 localPosition() const;
  glm::vec3 localScale() const;
  glm::quat localOrientation() const;


  glm::vec3 forward();
  glm::vec3 position() const;

  void collectParents(std::vector<std::shared_ptr<NodeTransform>> &parents) const;

  glm::quat orientation() const;

  std::shared_ptr<NodeTransform> parent() const;

private:
  glm::vec3 _position = {0, 0, 0} ;
  glm::vec3 _localPosition = {0, 0, 0};
  glm::quat _orientation = {0, 0, 0, 1};
  glm::quat _localOrientation = {0, 0, 0, 1};
  glm::vec3 _scale = {1, 1, 1};
  glm::vec3 _localScale = {1, 1, 1};

  glm::vec3 _forward = {0, 0, -1};
  glm::vec3 _right = {1, 0, 0};
  glm::vec3 _up = {0, 1, 0};

  std::shared_ptr<NodeTransform> _parent;

};

/**
* SceneNodes can have components, which are invoked once per frame.
* Components can be subclassed, and also "freeform" script components exist, which allow
* to attach arbitrary logic to a SceneNode.
*/
class NodeComponent {
  friend class SceneNode;
public:
  virtual void invoke() = 0;

  std::weak_ptr<SceneNode> getNode();

  void disable();
  void enable();


private:
  void setNode(std::weak_ptr<SceneNode> node);
  std::weak_ptr<SceneNode> node;
  bool enabled = true;
};

/**
* Subclass from ScriptComponent to have any code to implement logic in your
* SceneNodes.
*/
class ScriptComponent : public NodeComponent {
public:
  void invoke() override;

protected:
  virtual void update() = 0;


};



#endif //NODECOMPONENT_H
