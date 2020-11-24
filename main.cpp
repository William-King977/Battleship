#include "battleship.hpp"
#include <iostream>
#include <exception>
using namespace std;

int main(void) {
    // __cplusplus
    Battleship myGame;
    myGame.startGame();

    while (!myGame.isGameFinished()) {
        myGame.showBoard();

        string xy;
        cout << "Enter the co-ordinates (e.g. A1): ";
        getline(cin, xy);

        char x = xy[0];
        string strY;

        try {
            // Check if x is in range.
            if (x < 'A' || x > 'J')
                throw logic_error("The x-coordinate is out of range. Enter between A and J.");

            // Check the length.
            switch (xy.length()) {
                case 3:
                    strY = xy.substr(1,2);
                    break;
                case 2:
                    strY = xy.substr(1,1);
                    break;
                default:
                    throw logic_error("Invalid co-ordinate length.");
            }

            // Check if the y-coordinates are integers.
            for (char letter : strY) {
                if (letter < '0' || letter > '9')
                    throw logic_error("The y-coordinate must be in numbers.");
            }

            // Convert y-coordinates to an integer, then check the range.
            int y = stoi(strY);
            if (y < 0 || y > 10)
                throw logic_error("The y-coordinate is out of range. Enter between 1 and 10.");

            myGame.shoot(x, y);
        } catch (logic_error e) {
            cout << "Error: " << e.what() << endl;
        }
    } 
    return 0;
}