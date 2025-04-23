//
// Created by mgrus on 10.04.2025.
//

#ifndef AREAWIDGET_H
#define AREAWIDGET_H

#include <engine/graphics/Widget.h>

/**
* A container widget which has 5 children with specific positioning:
* - top, bottom, left, right, center
* - the size of the center is determined by the remainder of what is left by the other children.
* As a convenience, it sets itself an AreaLayout and some helper classes for dragging edges.
*/
class AreaWidget : public Widget {

public:
    AreaWidget();

    std::shared_ptr<Edge> createEdge(AreaLayoutPosition layoutPosition);

    void createPlaceholdersForNullParts();

    AreaWidget(std::shared_ptr<Widget> top, std::shared_ptr<Widget> bottom, std::shared_ptr<Widget> left, std::shared_ptr<Widget> right, std::shared_ptr<Widget> center);
    void draw(float depth) override;


    std::shared_ptr<Widget> top_;
    std::shared_ptr<Widget> bottom_;
    std::shared_ptr<Widget> left_;
    std::shared_ptr<Widget> center_;
    std::shared_ptr<Widget> right_;
    std::shared_ptr<EdgeDragger> edge_dragger_top_;
    std::shared_ptr<EdgeDragger> edge_dragger_bottom_;
    std::shared_ptr<EdgeDragger> edge_dragger_left_;
    std::shared_ptr<EdgeDragger> edge_dragger_right_;
    std::shared_ptr<EdgePainter> edge_painter_;
    std::shared_ptr<Edge> edge_top;
    std::shared_ptr<Edge> edge_bottom;
    std::shared_ptr<Edge> edge_left;
    std::shared_ptr<Edge> edge_right;
};



#endif //AREAWIDGET_H
