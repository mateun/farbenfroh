//
// Created by mgrus on 09.04.2025.
//

#ifndef EDGEPAINTER_H
#define EDGEPAINTER_H

#include <memory>
#include <vector>

struct Edge;
class EdgeDragger;

/**
* Paints edges in
* - hover
* - drag
* - normal
* modes.
*/
class EdgePainter {

  public:
    void addEdge(std::shared_ptr<Edge> edgeDragger);
    void draw(float depth);

private:
    std::vector<std::shared_ptr<Edge>> edges_;

};



#endif //EDGEPAINTER_H
