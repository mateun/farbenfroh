//
// Created by mgrus on 10.03.2025.
//

#include "QuadTree.h"

QuadTree::QuadTree(glm::vec2 center, float halfSize, int maxNodesPerTree) : center(center), halfSize(halfSize), maxNodesPerTree(maxNodesPerTree) {
}

/**
* Builds the actual tree structure based on the incoming nodes.
*/
void QuadTree::buildForObjects(const std::vector<TreeObj *> &inputNodes) {
    // First copy all nodes into our own working list,
    // where we can add & remove at our will:
    std::vector<TreeObj*> workingList;
    for (auto node : inputNodes) {
        workingList.push_back(node);
    }

    // Now for the actual processing.
    // We go through the working list and decide if the current node fits into ourselves.
    // If it does, we check if we already exceed our maximum number of nodes we can have in one tree.
    // If we do, we must subdivide ourselves.
    // Otherwise we can just add the node to our internal node list and move on.
    for (auto node : workingList) {
        glm::vec2 nodeLocation = {node->position.x, node->position.y};
        auto qt = findContainingLeafQuadTree(nodeLocation);
        if (qt) {
            qt->assignObject(node);
        }
    }
}

void QuadTree::insertObject(TreeObj* obj) {
    auto qt = findContainingLeafQuadTree(obj->position);
    qt->assignObject(obj);
}

std::vector<TreeObj*> QuadTree::getNodes() const {
    return nodes;
}

bool QuadTree::hasChildren() const {
    return topLeft != nullptr;
}


/**
* This finds the first (top..) quad-tree which contains the point.
* If no quadtree is found (out-of-bounds), then nullptr is returned.
*/
QuadTree* QuadTree::findFirstContainingQuadTree(glm::vec2 point) {
    if (contains({point.x, point.y})) {
        return this;
    }

    if (hasChildren()) {
        topLeft->findFirstContainingQuadTree(point);
        topRight->findFirstContainingQuadTree(point);
        bottomLeft->findFirstContainingQuadTree(point);
        bottomRight->findFirstContainingQuadTree(point);
    }

    return nullptr;

}

const std::vector<QuadTree *> QuadTree::childrenList() {
    return {topLeft, topRight, bottomLeft, bottomRight};
}

/**
 * This goes as deep into the tree as possible and finds the most tight fitting quadtree
 * we have.
 */
QuadTree * QuadTree::findContainingLeafQuadTree(glm::vec2 point) {
    if (hasChildren()) {
        for (auto qt : childrenList()) {
            auto found = qt->findContainingLeafQuadTree(point);
            if (found) {
                return found;
            }
        }
    } else {
        if (contains({point.x, point.y})) {
            return this;
        }
    }
    return nullptr;
}

float QuadTree::getHalfSize() const {
    return halfSize;
}

float QuadTree::getLeft() const {
    return center.x - halfSize;
}

float QuadTree::getRight() const {
    return center.x + halfSize;
}

float QuadTree::getTop() const {
    return center.y - halfSize;

}

float QuadTree::getBottom() const {
    return center.y + halfSize;
}

/**
 * This method does make an immediate subdivision if insertion fails due
 * to criteria such as too many objects already here.
 * It does NOT search for the best fitting leaf node - it just tries an insert,
 * and subdivides on failure.
 * It is only meant for internal usage.
 * Consider using insertObject>
 */
void QuadTree::assignObject(TreeObj *node) {
    if (nodes.size() < maxNodesPerTree || subdivisions > 3) {
        nodes.push_back(node);
    } else {
        subdivide(node);
    }

}

/**
* We found an object which triggers subdivision.
* Subdivision means we create 4 new QuadTree objects and assign them
* to our child slots.
*/
void QuadTree::subdivide(TreeObj* node) {
    subdivisions += 1;

    glm::vec2 centerTopLeft = {center.x - halfSize/2, center.y - halfSize/2};
    topLeft = new QuadTree(centerTopLeft, halfSize/2);
    topLeft->subdivisions = subdivisions;
    glm::vec2 centerTopRight = {center.x + halfSize/2, center.y - halfSize/2};
    topRight = new QuadTree(centerTopRight, halfSize/2);
    topRight->subdivisions = subdivisions;
    glm::vec2 centerBotLeft = {center.x - halfSize/2, center.y + halfSize/2};
    bottomLeft = new QuadTree(centerBotLeft, halfSize/2);
    bottomLeft->subdivisions = subdivisions;
    glm::vec2 centerBotRight = {center.x + halfSize/2, center.y + halfSize/2};
    bottomRight = new QuadTree(centerBotRight, halfSize/2);
    bottomRight->subdivisions = subdivisions;


    // First treat all existing nodes. We want to push them always as much down the
    // hierarchy as we can so we check if we find a new and more precise "home" for each node here.
    for (auto existingChild : nodes) {
        existingChild->position;
        for (auto qt : childrenList()) {
            if (qt->contains(existingChild->position)) {
                qt->assignObject(existingChild);
                break;
            }
        }

    }

    // Now find the containing child tree to add the node to:
    glm::vec2 nodeLocation = {node->position.x, node->position.y};
    for (auto qt : childrenList()) {
        auto found = qt->contains(nodeLocation);
        if (found) {
            qt->assignObject(node);
            break;
        }
    }
}

bool QuadTree::contains(glm::vec2 point) {
    return point.x > getLeft() && point.x < getRight() &&
        point.y < getBottom() && point.y > getTop();
}
