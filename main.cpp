#include <iostream>
#include "battleship.hpp"
using namespace std;

int main(void) {
    // __cplusplus;
    Battleship myShip;
    myShip.startGame();
    myShip.showBoard();
    return 0;
}