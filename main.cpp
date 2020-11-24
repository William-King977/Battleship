#include "battleship.hpp"
#include <iostream>
#include <cstdlib>
using namespace std;

int main(void) {
    // __cplusplus;
    Battleship myShip;
    myShip.startGame();

    while (!myShip.isGameFinished()) {
        myShip.showBoard();

        string xy;
        cout << "Enter the co-ordinates (e.g. A1): ";
        getline(cin, xy);

        char x = xy[0];
        string y;

        switch (xy.length()) {
            case 3:
                y = xy.substr(1,2);
                break;
            case 2:
                y = xy.substr(1,1);
            default:
                // Must be dealt with.
                cout << "Error" << endl;
        }

        myShip.shoot(x, y);
    }
    return 0;
}