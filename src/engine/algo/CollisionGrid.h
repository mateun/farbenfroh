//
// Created by mgrus on 11.03.2025.
//

#ifndef COLLISIONGRID_H
#define COLLISIONGRID_H
#include <glm/glm.hpp>
#include <vector>

enum class GridOrigin { Center, BottomLeft };
class PositionProvider;


class GridCell {
public:
    GridCell(glm::vec2 gridCoordinate);
    void assignObject(PositionProvider* positionProvider);

private:
    glm::vec2 gridCoordinate;
    std::vector<PositionProvider*> objects;

};

class CollisionGrid {

public:
    CollisionGrid(glm::vec2 worldPosition, GridOrigin origin, int numberHorizontalCells, int numberVerticalCells, int cellSize, std::vector<PositionProvider*> objects);

    /**
    * Re-Assigns all objects to the most current cells by
    * asking each object for its current position.
    */
    void refresh();

    /**
    * Assign (additional objects) to the respective cells.
    */
    void addObjects(std::vector<PositionProvider*> objects);


    /**
    * Finds the cell which bounds the given point.
    */
    GridCell* getCellForPosition(glm::vec2 point) const;

    glm::ivec2 worldToGridCoordinate(glm::vec2 worldCoordinate) const;

private:
    std::vector<std::vector<GridCell*>> cells;
    int cellSize;
    GridOrigin origin;
    glm::vec2 worldPosition;
    int number_horizontal_cells;
    int number_vertical_cells;
};



#endif //COLLISIONGRID_H
