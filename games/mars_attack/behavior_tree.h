//
// Created by mgrus on 16.01.2024.
//

#ifndef SIMPLE_KING_BEHAVIOR_TREE_H
#define SIMPLE_KING_BEHAVIOR_TREE_H

#include <vector>
#include <functional>

/**
 * This holds the maps for various datatypes
 * which are read and written during
 * BehaviorTreeExecution.
 */
struct BTBlackboard {
    std::map<std::string, std::string> strings;
    std::map<std::string, int> ints;
    std::map<std::string, float> floats;
    std::map<std::string, glm::vec3> vec3s;
    std::map<std::string, void*> objects;
};

struct BTNodeResult {
    bool success = false;
};

enum class BTNodeType {
    SEQUENCE,       // Every child task is executed from left to right
    SELECTOR,       // Only the first child task succeeding is run
    DECORATOR,      // e.g. IF
    TASK,           // Doing the actual work, e.g. moving the entity etc.

};

struct BTNode {
    BTNodeType type = BTNodeType::SEQUENCE;
    std::vector<BTNode> childNodes;
    std::function<BTNodeResult(BTBlackboard*)> taskFunction;
};

class BehaviorTree {

public:
    BehaviorTree();

    void execute();
    BTNodeResult executeNode(BTNode node);
    BTNodeResult executeTaskNode(BTNode node);
    BTNodeResult executeSequenceNode(BTNode node);

    BTBlackboard* blackboard = nullptr;
    BTNode root;

private:


    BTNodeResult executeSelectorNode(BTNode node);
};

#endif //SIMPLE_KING_BEHAVIOR_TREE_H
