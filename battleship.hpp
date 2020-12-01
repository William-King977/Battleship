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
    string name;
    int length;
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
        void startGame(int numPlayers, bool loadP1ShipFile, bool loadP2ShipFile);
        void showBoard();
        void shoot(char charX, int y);
        bool isGameFinished() { return isFinished; }
        int getNumPlayers() { return numPlayers; }
        int getCurrPlayer() { return currPlayer; }
    private:
        // static makes it useable in switch, case.
        static const char emptySpace = '-';
        int numPlayers;
        int currPlayer;
        char** p1Board;
        char** p2Board;
        int p1ShipCount;
        int p2ShipCount;
        bool isFinished;
        Ship prevShipHit;
        unordered_map<char, Ship> p1Ships;
        unordered_map<char, Ship> p2Ships;
        unordered_map<string, queue<Coordinate>> shipPosFound; // Discovered ship positions.
        unordered_map<string, queue<Coordinate>> cpuMoves; // Moves to sink the ship(s) found.

        // Methods.
        void placeShips(char** &board, unordered_map<char, Ship> &ships);
        void getShipsFromFile(string fileName, char** &currBoard);
        void setShipData(unordered_map<char, Ship> &ships);
        vector<direction> getDirections(int x, int y, int shipLength, char** board);
        void enemyShoot();
        void setCpuMoves(int x, int y, Ship thatShip);
        void backTrackShot(int x, int y);
        void setAltMoves(direction dir, int x, int y, Coordinate prevShipMove);
        void setPrevShip();
        bool isPosHit(int x, int y);
};

#endif