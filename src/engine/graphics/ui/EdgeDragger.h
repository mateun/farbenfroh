//
// Created by mgrus on 09.04.2025.
//

#ifndef EDGEDRAGGER_H
#define EDGEDRAGGER_H

#include <memory>
#include <glm/glm.hpp>

#include "FrameMessageSubscriber.h"

struct UIMessage;

enum class EdgeDirection {
  Vertical,
  Horizontal
};

struct Edge {
  EdgeDirection direction = EdgeDirection::Vertical;
  glm::vec2 origin = {0, 0};
  float size = 0;
};


/**
* This class allows the dragging of virtual edges.
* These edges can e.g. be formed by a splitted widget with two parts.
* Or represent an edge of an area in an AreaLayout.
* This class just gets to know about an edge and then it manages, within constraints (optional)
* the dragging/moving of this edge.
*
*/
class EdgeDragger : public FrameMessageSubscriber, public std::enable_shared_from_this<EdgeDragger> {

public:
  EdgeDragger(Edge edge, float zValue);

  void onFrameMessages(const std::vector<RawWin32Message>& msgs) override;

  // Retrieve the current origin of the managed edge,
  // may change over time if edge is dragged.
  glm::vec2 getOrigin();

private:
  Edge edge_;
  float zValue_ = 0;
  bool hover_ = false;
};



#endif //EDGEDRAGGER_H
