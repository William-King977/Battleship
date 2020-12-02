#ifndef SHIP_HPP
#define SHIP_HPP

#include <string>
using namespace std;

// Holds data (and status) of each ship.
class Ship {
    private:
        string name;
        int length;
        int health;
    
    public:
        Ship() : name(""), length(0), health(0) { }
        Ship(string name, int length, int health) : name(name), length(length), health(health) { } 

        string getName() { return name; }
        int getLength() { return length; }
        int getHealth() { return health; }

        void setName(string name) { this->name = name; }
        void setLength(int length) { this->length = length; }
        void setHealth(int health) { this->health = health; }
};

#endif