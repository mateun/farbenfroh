//
// Created by mgrus on 16.01.2024.
//
#include "behavior_tree.h"

static BTNodeResult executeNode(BTNode node);

BTNodeResult BehaviorTree::executeTaskNode(BTNode node) {
    if (node.taskFunction) {
        return {node.taskFunction(blackboard)};
    } else {
        return {false};
    }
}

BTNodeResult BehaviorTree::executeSequenceNode(BTNode node) {
    // Execute every child regardless of the result
    for (auto c : node.childNodes) {
        executeNode(c);
    }
    return {true};
}

/**
 * Selects the first node wich is successful.
 * @param node The selector node
 * @return success if we found at least one success task or failure otherwise.
 */
BTNodeResult BehaviorTree::executeSelectorNode(BTNode node) {
    for (auto c : node.childNodes) {
        auto result = executeNode(c);
        if (result.success) {
            return result;
        }
    }
    return {false};
}

/**
 * Generic node execution function.
 * This dispatches to the actual node types.
 * @param node The node to execute
 * @return the result of the node execution.
 */
BTNodeResult BehaviorTree::executeNode(BTNode node) {

    if (node.type == BTNodeType::TASK) {
        return executeTaskNode(node);
    }
    else if (node.type == BTNodeType::SEQUENCE) {
        return executeSequenceNode(node);
    } else if (node.type == BTNodeType::SELECTOR) {
        return executeSelectorNode(node);
    }
    else return {false};

}

void BehaviorTree::execute() {
    auto result = executeNode(root);

}

BehaviorTree::BehaviorTree() {
    BTBlackboard* bb = new BTBlackboard();
    blackboard = bb;
}

