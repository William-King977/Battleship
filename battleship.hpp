#ifndef BATTLESHIP_HPP
#define BATTLESHIP_HPP

#include "ship.hpp"
#include "coordinate.hpp"
#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
using namespace std;
enum Direction {UP, DOWN, LEFT, RIGHT};

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
        // Ship placements.
        void placeShips(char** &board, unordered_map<char, Ship> &ships);
        void getShipsFromFile(string fileName, char** &currBoard);
        void setShipData(unordered_map<char, Ship> &ships);
        vector<Direction> getValidDirections(int x, int y, int shipLength, char** board);

        // CPU methods.
        void cpuShoot();
        void setCpuMoves(int x, int y, Ship thatShip);
        void backTrackShot(int x, int y);
        void setAltMoves(Direction dir, Coordinate prevShipMove);
        void setPrevShip();
        Direction getDirection(Coordinate first, Coordinate last);
        bool canShipExist(int shipLength, Coordinate currPos, Direction dir);
        bool isPosHit(int x, int y);
};

#endif