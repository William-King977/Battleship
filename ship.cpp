#include "ship.hpp"

// Constructor with no parameters.
Ship::Ship() {
    this->name = "";
    this->length = 0;
    this->health = 0;
}

// Constructor with all parameter.
Ship::Ship(string name, int length, int health) {
    this->name = name;
    this->length = length;
    this->health = health;
}

// Deconstructor.
Ship::~Ship() { }

// Getters for each variable.
string Ship::getName() { 
    return name; 
}
int Ship::getLength() { 
    return length; 
}
int Ship::getHealth() { 
    return health; 
}

// Setters.
void Ship::setName(string name) { 
    this->name = name; 
}       
void Ship::setHealth(int health) { 
    this->health = health;
}