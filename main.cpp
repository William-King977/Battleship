#include "battleship.hpp"
#include <iostream>
#include <exception>
using namespace std;

int main(void) {
    cout << "----------------------Battleship--------------------" << endl;
    
    // Ask for the number of players.
    string strNumPlayers;
    int numPlayers;
    bool validNumPlayers = false;

    while (!validNumPlayers) {
        try {
            cout << "Enter the number of players (1 or 2): ";
            getline(cin, strNumPlayers);

            // Check the length.
            if (strNumPlayers.length() > 1)
                throw logic_error("Invalid input, please enter a single digit.");
            
            if (strNumPlayers.length() == 0)
                throw logic_error("No option entered.");

            // Check for a zero.
            if (strNumPlayers[0] == '0') 
                throw logic_error("You can't have no players.");
            
            // If a non-digit is entered.
            if (strNumPlayers[0] < '1' || strNumPlayers[0] > '9') 
                throw logic_error("Only digits are allowed.");

            // Check if a one or two is entered.
            switch (strNumPlayers[0]) {
                case '1':
                    numPlayers = 1;
                    validNumPlayers = true;
                    break;
                case '2':
                    numPlayers = 2;
                    validNumPlayers = true;
                    break;
                default:
                    throw logic_error("Too many players, it must be 1 or 2.");
            }
        } catch (logic_error e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    // Asks the user if they want to read their ship placements from their file.
    string shipOption;
    bool shipFromFile;
    bool loadP1ShipFile = false;
    bool loadP2ShipFile = false;

    for (int i = 0; i < numPlayers; i++) {
        bool validOption = false;
        while (!validOption) {
            cout << "Load pre-defined ships for Player " << i + 1 << " (Y/N)? ";
            getline(cin, shipOption);

            try {
                // Check input length.
                if (shipOption.length() > 1)
                    throw logic_error("Invalid option, input is too long.");

                if (shipOption.length() == 0)
                    throw logic_error("No option entered.");

                // Check the option entered.
                switch (shipOption[0]) {
                    case 'N':
                    case 'n':
                        shipFromFile = false;
                        validOption = true;
                        break;
                    case 'Y':
                    case 'y':
                        shipFromFile = true;
                        validOption = true;
                        break;
                    default:
                        throw logic_error("Invalid option, enter Y or N.");
                }
            } catch (logic_error e) {
                cout << "Error: " << e.what() << endl;
            }
        }
        loadP1ShipFile = (i == 0) ? shipFromFile : loadP1ShipFile;
        loadP2ShipFile = (i == 1) ? shipFromFile : loadP2ShipFile;
    }

    // Initialise the game.
    Battleship myGame;

    // Stop the game if the file can't be found (if they choose to use it).
    try {
        myGame.startGame(numPlayers, loadP1ShipFile, loadP2ShipFile);
    } catch (runtime_error e) {
        cout << "Error: " << e.what() << endl;
        cout << "Terminating game." << endl;
        return -1;
    }

    // Run the game until it's finished.
    while (!myGame.isGameFinished()) {
        // Display text depending on the player turn and number of players.
        if (numPlayers == 2) {
            if (myGame.getCurrPlayer() == 1) {
                cout << endl << "-----------------------P1 Turn----------------------" << endl;
            } else {
                cout << endl << "-----------------------P2 Turn----------------------" << endl;
            }
        } else {
            cout << endl << "----------------------Your Turn---------------------" << endl;
        }
        
        myGame.showBoard();

        string xy;
        cout << "Enter the co-ordinates (e.g. A1): ";
        getline(cin, xy);

        char x;
        string strY;

        try {
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
            
            // Set x here (it's possible to input nothing).
            x = xy[0];

            // Check if x is in range.
            if (x < 'A' || x > 'J')
                throw logic_error("The x-coordinate is out of range. Enter between A and J.");

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