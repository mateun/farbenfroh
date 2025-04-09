//
// Created by mgrus on 09.04.2025.
//

#include "TreeViewer.h"

#include <engine/graphics/Renderer.h>

#include "LabelWidget.h"

TreeNode::TreeNode(std::shared_ptr<void> userData): user_data_(userData) {

}

std::string TreeNode::getText() {
    return text_;
}

void TreeNode::setText(const std::string &text) {
    text_ = text;
}

std::vector<std::shared_ptr<TreeNode>> TreeNode::childNodes() {
    return children_;
}

void TreeNode::addChild(std::shared_ptr<TreeNode> childNode) {
    children_.push_back(childNode);
}

TreeModel::TreeModel(std::shared_ptr<TreeNode> root): root_(std::move(root)) {
}

std::shared_ptr<TreeNode> TreeModel::getRoot() {
    return root_;
}

TreeViewer::TreeViewer(std::shared_ptr<TreeModel> treeModel): tree_model_(treeModel) {
    label_widget_ = std::make_shared<LabelWidget>("--", getApplication()->getDefaultMenuFont());
}

void TreeViewer::renderNode(glm::vec2 position, const std::shared_ptr<TreeNode> &node, float renderDepth) {
    auto label_widget = std::make_shared<LabelWidget>(node->getText(), getApplication()->getDefaultMenuFont());
    label_widget->setText(node->getText());
    label_widget->setOrigin(origin() + position);
    label_widget->setSize(label_widget->getPreferredSize());
    label_widget->draw(renderDepth);


}

// For each node, we render
// - an optional icon
// - a mandatory text
// - if a node has children, we render a "plus" icon for expansion
void TreeViewer::draw(float depth) {

    auto root_node = tree_model_->getRoot();
    renderNode(glm::vec2(16, size().y - 32), root_node, depth + 0.01);

    // render top level nodes
    float yLoc = size().y - 48;
    for (auto tln : root_node->childNodes()) {
        renderNode(glm::vec2(32, yLoc), tln, depth + 0.01);
        yLoc -= 16;
    }

}
