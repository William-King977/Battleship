#ifndef BATTLESHIP_HPP
#define BATTLESHIP_HPP

#include <vector>
using namespace std;
enum direction {UP, DOWN, LEFT, RIGHT};

class Battleship {
    public:
        Battleship();
        ~Battleship();
        void startGame();
        void showBoard();
    private:
        char** p1Board;
        void placeShips(char emptySpace);
        vector<direction> getDirections(int x, int y, char emptySpace, int shipLength);
};

#endif