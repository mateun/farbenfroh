//
// Created by mgrus on 10.03.2025.
//

#include <engine/algo/QuadTree.h>

void whenHavingOneNodeItIsContainedInTheTopLevelQuadtree() {
    auto qt = new QuadTree({0, 0}, 100);
    TreeObj obj1 = {{-2, -2}, 2};

    qt->buildForObjects({&obj1});
    auto foundQt = qt->findFirstContainingQuadTree(obj1.position);
    assert(foundQt != nullptr);

    foundQt = qt->findContainingLeafQuadTree(obj1.position);
    assert(foundQt != nullptr);
    assert(foundQt != qt);

}

void deepRecursion() {
    TreeObj obj1 = {{-2, -2}, 2};
    TreeObj obj2 = {{-9, -4}, 2};
    TreeObj obj3 = {{-11, -15}, 2};
    TreeObj obj4 = {{-13, -17}, 2};

    auto qt2 = new QuadTree({0, 0}, 100);
    qt2->buildForObjects({&obj1, &obj2, &obj3, &obj4});
    auto foundQt = qt2->findFirstContainingQuadTree(obj2.position);
    assert(foundQt != nullptr);
    assert(foundQt->getHalfSize() == 100);

    foundQt = qt2->findContainingLeafQuadTree(obj4.position);
    assert(foundQt->getHalfSize() == 6.25);

    foundQt = qt2->findContainingLeafQuadTree(obj1.position);
    assert(foundQt->getHalfSize() == 6.25);
    auto nodes = foundQt->getNodes();
    assert(nodes.size() == 2);


    TreeObj obj5 = {{-3, -5}, 2};
    foundQt = qt2->findContainingLeafQuadTree(obj5.position);
    foundQt->insertObject(&obj5);
    foundQt = qt2->findContainingLeafQuadTree(obj5.position);
    nodes = foundQt->getNodes();
    assert(nodes.size() == 3);

}

void expectingSubidvidedQuadTreeToContainNode() {
    TreeObj obj1 = {{-2, -2}, 2};
    TreeObj obj2 = {{-9, -4}, 2};
    TreeObj obj3 = {{75, -75}, 2};
    TreeObj obj4 = {{80, -60}, 2};
    TreeObj obj5 = {{85, -95}, 2};
    TreeObj obj6 = {{90, -98}, 2};

    auto qt2 = new QuadTree({0, 0}, 100);
    qt2->buildForObjects({&obj1, &obj2, &obj3, &obj4, &obj5, &obj6});
    auto foundQt = qt2->findFirstContainingQuadTree(obj2.position);
    assert(foundQt != nullptr);
    assert(foundQt->getHalfSize() == 100);

    foundQt = qt2->findContainingLeafQuadTree(obj2.position);
    assert(foundQt->getHalfSize() == 50);
    assert(foundQt->getLeft() == -100);
    assert(foundQt->getRight() == 0);
    assert(foundQt->getBottom() == 0);
    assert(foundQt->getTop() == -100);

    foundQt = qt2->findContainingLeafQuadTree(obj3.position);
    assert(foundQt->getHalfSize() == 50);
    assert(foundQt->getLeft() == 0);
    assert(foundQt->getRight() == 100);
    assert(foundQt->getBottom() == 0);
    assert(foundQt->getTop() == -100);

}

#ifdef QUADTREE_TEST_ENABLED
int main(int argc, char** argv) {
    // whenHavingOneNodeItIsContainedInTheTopLevelQuadtree();
    // expectingSubidvidedQuadTreeToContainNode();
    deepRecursion();
    return 0;
}
#endif
