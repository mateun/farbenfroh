//
// Created by mgrus on 10.03.2025.
//

#ifndef QUADTREE_H
#define QUADTREE_H

#include <glm/glm.hpp>
#include <vector>

class TreeObj {
public:
  glm::vec2 position;
  float radius;
};

/**
* Separates the space into 4 quads (seen from the top)
* where each child contains 1 (or a small number) of nodes.
* This is efficient because for example when doing a bullet collision check, instead of
* going for every bullet against every enemy (or even other bullet..),
* the bullet, we just traverse the quadtree down from the top and find the lowest quadtree where the location of the
* bullet is contained. This should be only a few hops. And then we do the collision checks only against the
* node(s) within this leaf quadtree. So instead instead of M x N checks, we first hop a few times to find the
* correct quadtree and second we just check against the potential other nodes in the leaf quadtree.
*/
class QuadTree {

  public:
    QuadTree(glm::vec2 center, float halfSize, int maxNodesPerTree = 1);
    void buildForObjects(const std::vector<TreeObj*>& nodes);
    void insertObject(TreeObj* obj);


    QuadTree* findFirstContainingQuadTree(glm::vec2 point);
    QuadTree* findContainingLeafQuadTree(glm::vec2 point);

    bool hasChildren() const;
    float getHalfSize() const;
    float getLeft() const;
    float getRight() const;
    float getTop() const;
    float getBottom() const;
    std::vector<TreeObj*> getNodes() const;

  private:

    void assignObject(TreeObj *node);
    void subdivide(TreeObj* node);
    bool contains(glm::vec2 point);
    const std::vector<QuadTree*> childrenList();

    glm::vec2 center;
    float halfSize;

    std::vector<TreeObj*> nodes;

    QuadTree* topLeft = nullptr;
    QuadTree* topRight = nullptr;
    QuadTree* bottomLeft = nullptr;
    QuadTree* bottomRight = nullptr;
    int maxNodesPerTree;
    int subdivisions = 0;


};



#endif //QUADTREE_H
