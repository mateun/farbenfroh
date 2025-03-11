//
// Created by mgrus on 11.03.2025.
//

#include "CollisionGrid.h"

GridCell::GridCell(glm::vec2 gridCoordinate) : gridCoordinate(gridCoordinate) {
}

void GridCell::assignObject(PositionProvider *positionProvider) {
    objects.push_back(positionProvider);
}

void CollisionGrid::addObjects(std::vector<PositionProvider *> objects) {
}

GridCell * CollisionGrid::getCellForPosition(glm::vec2 point) const {
    auto gridCoord = worldToGridCoordinate(point);
    return cells[gridCoord.x][gridCoord.y];
}



CollisionGrid::CollisionGrid(glm::vec2 worldPosition, GridOrigin origin, int numberHorizontalCells, int numberVerticalCells, int cellSize, std::vector<PositionProvider *> objects) : cellSize(cellSize), origin(origin), worldPosition(worldPosition),
    number_horizontal_cells(numberHorizontalCells), number_vertical_cells(numberVerticalCells) {

    cells.resize(numberHorizontalCells);
    for (auto& c : cells) {
        c.resize(numberVerticalCells);
    }
    for (int i = 0; i < numberHorizontalCells; i++) {
        for (int j = 0; j < numberVerticalCells; j++) {
            auto cell = new GridCell({i, j});;
            cells[i][j] = cell;
        }
    }

    for (auto o : objects) {
        auto cell = getCellForPosition(o->getPosition());
        cell->assignObject(o);
    }
}

void CollisionGrid::refresh() {
}

glm::ivec2 CollisionGrid::worldToGridCoordinate(glm::vec2 worldCoordinate) const {
    int xWorldNormalizedByPivot = worldCoordinate.x + (number_horizontal_cells / 2 * cellSize);
    int yWorldNormalizedByPivot = worldCoordinate.y + (number_vertical_cells / 2 * cellSize);

    int x = xWorldNormalizedByPivot / cellSize;
    int y = yWorldNormalizedByPivot / cellSize;

    return {x, y};
}

