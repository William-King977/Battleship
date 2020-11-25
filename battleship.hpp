#ifndef BATTLESHIP_HPP
#define BATTLESHIP_HPP

#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
using namespace std;
enum direction {UP, DOWN, LEFT, RIGHT};

// Holds data (and status) of each ship.
struct Ship {
    string fullName;
    int health;
};

// Co-ordinates for possible CPU moves.
struct Coordinate {
    int x;
    int y;
    Coordinate(int newX, int newY) : x(newX), y(newY) { }
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
        // static makes it useable in switch, case.
        static const char emptySpace = '-'; 
        char** p1Board;
        char** p2Board;
        int p1ShipCount;
        int p2ShipCount;
        bool isFinished;
        unordered_map<char, Ship> p1Ships;
        unordered_map<char, Ship> p2Ships;
        queue<Coordinate> cpuMoves;

        // Methods.
        void placeShips(char** &board, unordered_map<char, Ship> &ships);
        vector<direction> getDirections(int x, int y, int shipLength, char** board);
        void enemyShoot();
        void setCpuMoves(int x, int y);
};

#endif