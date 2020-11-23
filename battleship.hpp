#ifndef BATTLESHIP_HPP
#define BATTLESHIP_HPP

class Battleship {
    public:
        Battleship();
        ~Battleship();
        void startGame();
        void showBoard();
    private:
        char** p1Board;
        void placeShips(char emptySpace);
};

#endif