#include "../include/battleship.hpp"
#include "../include/battleshipCPU.hpp"
#include <iostream>
#include <exception>
using namespace std;

void setNumPlayers(int&);
void setFileOptions(int, bool&, bool&);
void runGame(Battleship*);
void checkGameStatus(Battleship*);
void playAgain(void);

// DRIVER CODE.
int main(void) {
    cout << "-----------------------Battleship---------------------" << endl;
    
    // Ask for the number of players.
    int numPlayers;
    setNumPlayers(numPlayers);

    // Asks the user if they want to read their ship placements from their file.
    bool loadP1ShipFile = false;
    bool loadP2ShipFile = false;
    setFileOptions(numPlayers, loadP1ShipFile, loadP2ShipFile);

    // Initialise the game.
    Battleship* myGame = nullptr;
    if (numPlayers == 1) {
        myGame = new BattleshipCPU();
    } else {
        myGame = new Battleship();
    }

    // Restart the game if the file can't be found (if they choose to use it).
    try {
        myGame->startGame(numPlayers, loadP1ShipFile, loadP2ShipFile);
    } catch (runtime_error e) {
        cout << "Error: " << e.what() << endl;
        cout << "Restarting game..." << endl;
        // Delete current game object, then restart the game.
        delete myGame;
        main();
        return 0;
    }

    // Run the game until completion.
    runGame(myGame);
    playAgain();
    return 0;
}

// Sets the number of players for the game.
void setNumPlayers(int &numPlayers) {
    bool validNumPlayers = false;
    while (!validNumPlayers) {
        try {
            string strNumPlayers;
            cout << "Enter the number of players (1 or 2): ";
            getline(cin, strNumPlayers);

            // Check the length.
            if (strNumPlayers.length() > 1) {
                throw logic_error("Invalid input, please enter a single digit.");
            } else if (strNumPlayers.length() == 0) {
                throw logic_error("No option entered.");
            }

            // Check for a zero.
            if (strNumPlayers[0] == '0') {
                throw logic_error("You can't have no players.");
            }
            
            // If a non-digit is entered.
            if (strNumPlayers[0] < '1' || strNumPlayers[0] > '9')  {
                throw logic_error("Only digits are allowed.");
            }

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
                    throw logic_error("Too many players, it must be either 1 or 2.");
            }
        } catch (logic_error e) {
            cout << "Error: " << e.what() << endl;
        }
    }
}

// Sets whether the players want to use a pre-positioned file of ships or not.
void setFileOptions(int numPlayers, bool &loadP1ShipFile, bool &loadP2ShipFile) {
    string shipOption;
    bool shipFromFile;
    for (int currPlayer = 1; currPlayer <= numPlayers; currPlayer++) {
        bool validOption = false;
        // Putting it in a seperate loop prevents asking BOTH users again
        // when an invalid option is entered.
        while (!validOption) {
            cout << "Load pre-positioned ships for Player " << currPlayer << " (Y/N)? ";
            getline(cin, shipOption);

            try {
                // Check input length.
                if (shipOption.length() > 1) {
                    throw logic_error("Invalid option, input is too long.");
                } else if (shipOption.length() == 0) {
                    throw logic_error("No option entered.");
                }

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
        loadP1ShipFile = (currPlayer == 1) ? shipFromFile : loadP1ShipFile;
        loadP2ShipFile = (currPlayer == 2) ? shipFromFile : loadP2ShipFile;
    }
}

// Runs the game until it's finished.
void runGame(Battleship* myGame) {
    // Runs until all the ships have sunk for one side.
    while (!myGame->isGameFinished()) {
        for (int currPlayer = 1; currPlayer <= myGame->getNumPlayers(); currPlayer++) {
            // Display text depending on the player turn and number of players.
            switch (myGame->getNumPlayers()) {
                case 2:
                    cout << endl << "------------------------P" << currPlayer << " Turn-----------------------" << endl;
                    break;
                case 1:
                    cout << endl << "-----------------------Your Turn----------------------" << endl;
                    break;
            }
            
            myGame->showBoard();

            // Get co-ordinates, then seperate them.
            string xy;
            cout << "Enter the co-ordinates (e.g. A1): ";
            getline(cin, xy);

            char x;
            string strY;

            try {
                // Check the length.
                switch (xy.length()) {
                    // If y is 10.
                    case 3:
                        strY = xy.substr(1,2);
                        break;
                    // If y is between 1 and 9.
                    case 2:
                        strY = xy[1];
                        break;
                    default:
                        throw logic_error("Invalid co-ordinate length.");
                }
                
                // Set x here (it's possible to input nothing).
                x = toupper(xy[0]);

                // Check if x is in range.
                if (x < 'A' || x > 'J') {
                    throw logic_error("The x-coordinate is out of range. Enter between A and J.");
                }

                // Check if the y-coordinates are integers.
                for (char letter : strY) {
                    if (letter < '0' || letter > '9') {
                        throw logic_error("The y-coordinate must be in numbers.");
                    }
                }

                // Convert y-coordinates to an integer, then check the range.
                int y = stoi(strY);
                if (y < 1 || y > 10) {
                    throw logic_error("The y-coordinate is out of range. Enter between 1 and 10.");
                }

                myGame->shoot(x, y);
                // Run the CPU's turn if it's single player.
                if (myGame->getNumPlayers() == 1) {
                    cout << endl << "----------------------CPU's Turn----------------------" << endl;
                    static_cast<BattleshipCPU*>(myGame)->cpuShoot();
                }
            } catch (logic_error e) {
                currPlayer--; // It will run the FOR loop again.
                cout << "Error: " << e.what() << endl;
            }
        }
        // Check the game's status after both player's turns.
        checkGameStatus(myGame);
    }
    // Delete the game object after game completion.
    delete myGame;
}

// Check the game's status after both players have taken their turn.
void checkGameStatus(Battleship* myGame) {
    // Ajust the game status if a player has won.
    // The CPU is treated as Player 2.
    if (myGame->isP1Win() || myGame->isP2Win()) {
        cout << endl << "-----------------------Game Over----------------------" << endl;
        myGame->showBoard();
        myGame->setGameFinished(true);

        // Show a message based on the game's outcome.
        switch (myGame->getNumPlayers()) {
            case 2:
                if (myGame->isP1Win() && myGame->isP2Win()) {
                    cout << "All of Player 1's and Player 2's ships have sunk." << endl;
                    cout << "Draw!" << endl;
                } else {
                    char winnerNum = myGame->isP1Win() ? '1' : '2';
                    char loserNum = myGame->isP1Win() ? '2' : '1';
                    cout << "All of Player " << loserNum << "'s ships have sunk." << endl;
                    cout << "Player " << winnerNum << " wins!" << endl;
                }
                break;
            case 1:
                if (myGame->isP1Win() && myGame->isP2Win()) {
                    cout << "All of your ships and the CPU's ships have sunk." << endl;
                    cout << "Draw!" << endl;
                } else if (myGame->isP1Win()) {
                    cout << "All of the CPU's ships have sunk." << endl;
                    cout << "You win!" << endl;
                } else if (myGame->isP2Win()) {
                    cout << "All your ships have sunk." << endl;
                    cout << "The CPU wins!" << endl; 
                }
                break;
        }
    }
}

// Asks if the player wants to play again.
void playAgain(void) {
    string option;
    cout << "Do you want to play again (Y/N)? ";
    getline(cin, option);

    try {
        // Check input length.
        if (option.length() > 1) {
            throw logic_error("Invalid option, input is too long.");
        } else if (option.length() == 0) {
            throw logic_error("No option entered.");
        }

        // Check the option entered.
        switch (option[0]) {
            case 'N':
            case 'n':
                // Ends the program...
                cout << "Terminating game..." << endl;
                break;
            case 'Y':
            case 'y':
               // Runs the main method again.
                main();
                break;
            default:
                throw logic_error("Invalid option, enter Y or N.");
        }
    } catch (logic_error e) {
        cout << "Error: " << e.what() << endl;
        playAgain(); 
    }    
}