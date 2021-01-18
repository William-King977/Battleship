#ifndef BATTLESHIP_HPP
#define BATTLESHIP_HPP

#include "ship.hpp"
#include "coordinate.hpp"
#include <vector>
#include <unordered_map>
using namespace std;
enum Direction {UP, DOWN, LEFT, RIGHT};

class Battleship {
    public:
        Battleship();
        virtual ~Battleship(); // Virtual ensures subclass deconstructor runs as well.
        void startGame(int numPlayers, bool loadP1ShipFile, bool loadP2ShipFile);
        void showBoard();
        void setGameFinished(bool status) { isFinished = status; }
        void shoot(char charX, int y);
        bool isP1Win() { return p1Win; }
        bool isP2Win() { return p2Win; }
        bool isGameFinished() { return isFinished; }
        int getNumPlayers() { return numPlayers; }
        int getCurrPlayer() { return currPlayer; }
    protected:
        static const char emptySpace = '-'; // static makes it useable in switch, case.
        int numPlayers;
        int currPlayer;
        bool p1Win;
        bool p2Win;
        bool isFinished;

        char** p1Board;
        char** p2Board;
        int p1ShipCount;
        int p2ShipCount;
        unordered_map<char, Ship> p1Ships;
        unordered_map<char, Ship> p2Ships;

        // Methods.
        // Ship placements.
        void placeShips(char** board);
        void getShipsFromFile(string fileName, char** currBoard);
        void setShipData(unordered_map<char, Ship> &ships);
        bool isShipPlacementValid(char** board);
        bool isShipValid(char** board, vector<Coordinate> &shipPos, char shipType, int shipLength);
        vector<Direction> getValidDirections(int x, int y, int shipLength, char** board);

        // Other
        bool isPosHit(char boardPiece);
};

#endif