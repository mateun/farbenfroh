//
// Created by mgrus on 10.03.2025.
//

#include <vector>
#include <engine/algo/CollisionGrid.h>
#include <engine/algo/PositionProvider.h>
#include <glm/vec2.hpp>

class MyPosProvider : public PositionProvider {
public:
    MyPosProvider(glm::vec2 pos) : pos(pos) {}

    glm::vec2 getPosition() override {
        return pos;
    }

    glm::vec3 getPosition3D() override {
        return glm::vec3(pos.x, pos.y, 0);
    }
private:
    glm::vec2 pos;
};

void basicGridConstructionTest() {
    std::vector<PositionProvider*> objects;
    auto o1 = new MyPosProvider(glm::vec2(2, 1));
    auto o2 = new MyPosProvider(glm::vec2(-6, 6));
    objects.push_back(o1);
    objects.push_back(o2);

    auto grid = new CollisionGrid(glm::vec2{0, 0}, GridOrigin::Center, 10,10, 5, objects);

    auto cell = grid->getCellForPosition({-6, 6});
    assert(cell != nullptr);
    // TODO test for objects in this cell. assert(cell.)

}


#ifdef COLL_GRID_TESTS
int main(int argc, char** argv) {
    basicGridConstructionTest();
    return 0;
}
#endif
