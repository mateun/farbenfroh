//
// Created by mgrus on 17.11.2024.
//

#ifndef SIMPLE_KING_DATA_STRUCTURES_H
#define SIMPLE_KING_DATA_STRUCTURES_H

#include <iostream>


template <typename T>
class BinaryTree {
public:
    BinaryTree() : value(), left(nullptr), right(nullptr) {}
    explicit BinaryTree(const T& val) : value(val), left(nullptr), right(nullptr) {}


    // The new partial tree will be inserted according to its value
    void add(BinaryTree* t) {
        if (!t) {
            return;
        }

        if (t->value < value) {
            if (left) {
                left->add(t);
            } else {
                left = t;
            }
        } else {
            if (right) {
                right->add(t);
            } else {
                right = t;
            }
        }
    };

    void print() const {
        if (left) left->print();
        std::cout << value << " ";
        if (right) right->print();
        std::cout << std::endl;
    }


private:
    T value;

    BinaryTree* left;
    BinaryTree* right;
};

#endif //SIMPLE_KING_DATA_STRUCTURES_H
