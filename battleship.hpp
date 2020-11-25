#ifndef BATTLESHIP_HPP
#define BATTLESHIP_HPP

#include <vector>
#include <string>
using namespace std;
enum direction {UP, DOWN, LEFT, RIGHT};

// Holds data (and status) of each ship.
struct Ship {
    char codeName;
    string fullName;
    int health;
};

class Battleship {
    public:
        Battleship();
        ~Battleship();
        void startGame();
        void showBoard();
        void shoot(char charX, int y);
        bool isGameFinished();
    private:
        // static const'expr' makes it useable in switch, case.
        static constexpr char emptySpace = '-'; 
        char** p1Board;
        char** p2Board;
        vector<Ship> p1Ships;
        vector<Ship> p2Ships;
        int p1ShipCount;
        int p2ShipCount;
        bool isFinished;
        void placeShips(char** &board, vector<Ship> &ships);
        vector<direction> getDirections(int x, int y, int shipLength, char** board);
};

#endif