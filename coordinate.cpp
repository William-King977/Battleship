#include "coordinate.hpp"

// Constructor.
Coordinate::Coordinate(int x, int y) {
    this->x = x;
    this->y = y;
}

// Deconstructor.
Coordinate::~Coordinate() { }

// Getters.
int Coordinate::getX() {
    return x;
}
int Coordinate::getY() {
    return y;
}
