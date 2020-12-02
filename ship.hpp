#ifndef SHIP_HPP
#define SHIP_HPP

#include <string>
using namespace std;

// Holds data of a ship.
class Ship {
    private:
        string name;
        int length;
        int health;
    
    public:
        Ship();
        Ship(string name, int length, int health);
        ~Ship();

        // Getters.
        string getName();
        int getLength();
        int getHealth();

        // Setters.
        void setName(string name);
        void setLength(int length);
        void setHealth(int health);
};

#endif