//
// Created by mgrus on 09.04.2025.
//

#ifndef TREEVIEWER_H
#define TREEVIEWER_H

#include <engine/graphics/Widget.h>

#include "LabelWidget.h"


class TreeNode {
public:
  TreeNode(std::shared_ptr<void> userData);
  virtual std::string getText();

  void setText(const std::string& text);

  std::vector<std::shared_ptr<TreeNode>> childNodes();

  void addChild(std::shared_ptr<TreeNode> childNode);

protected:
  std::shared_ptr<void> user_data_;
  std::string text_;
  std::vector<std::shared_ptr<TreeNode>> children_;
};

class TreeModel {
public:
  TreeModel(std::shared_ptr<TreeNode> root);
  std::shared_ptr<TreeNode> getRoot();

private:
  std::shared_ptr<TreeNode> root_;


};

/**
* This class renders a tree of hierarchical data.
* The data must be in the form of a <code>TreeModel</code> implementation
* which is consulted during the tree rendering process.
*
*/
class TreeViewer : public Widget {

public:
  TreeViewer(std::shared_ptr<TreeModel> treeModel);

  void renderNode(glm::vec2 position, const std::shared_ptr<TreeNode> & node, float renderDepth);

  void draw(float depth) override;

private:
  std::shared_ptr<TreeModel> tree_model_;
  std::shared_ptr<LabelWidget> label_widget_;
  std::vector<std::shared_ptr<LabelWidget>> frame_life_labels_;
};



#endif //TREEVIEWER_H
