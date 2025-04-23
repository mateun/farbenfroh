//
// Created by mgrus on 09.04.2025.
//

#include "EdgePainter.h"
#include "EdgeDragger.h"

void EdgePainter::addEdge(std::shared_ptr<Edge> edge) {
    edges_.push_back(edge);
}

void EdgePainter::draw(float depth) {
}
