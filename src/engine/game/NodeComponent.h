//
// Created by mgrus on 19.03.2025.
//

#ifndef NODECOMPONENT_H
#define NODECOMPONENT_H

enum class Space {
  WORLD,
  LOCAL,
};

class NodeTransform {
  friend class NodeComponent;
public:

  glm::vec3 localPosition();
  glm::vec3 localScale();
  glm::vec3 localOrientation();
  glm::vec3 worldPosition();
  glm::vec3 localForward();

private:
  glm::vec3 _position;
  glm::quat _orientation;
  glm::vec3 _scale;
  glm::vec3 _forward;
  glm::vec3 _right;
  glm::vec3 _up;

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

  // Some shortcut convenience functions which allow acces to the node transform
  NodeTransform getTransform();
  void setPosition(glm::vec3 position);

  void disable();


private:
  void setNode(SceneNode* node) {
    this->node = node;
  }

  SceneNode* node = nullptr;

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
