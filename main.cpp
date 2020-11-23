#include <iostream>
#include "battleship.hpp"
using namespace std;

int main(void) {
    Battleship myShip;
    myShip.startGame();
    myShip.showBoard();
    return 0;
}