#include "battleship.hpp"
#include <iostream>
#include <exception>
#include <cstdlib>
#include <ctime>

Battleship::Battleship() {
    // cout << "Battleship object made." << endl;
}

Battleship::~Battleship() {
    for (int i = 0; i < 10; i++) {
        delete[] p1Board[i];
        delete[] p2Board[i];
    }
    delete[] p1Board;
    delete[] p2Board;
}

// Initialises the game components and fills the board.
void Battleship::startGame() {
    char emptySpace = '~';
    p1Board = new char* [10];
    p2Board = new char* [10];

    p1ShipCount = 5;
    p2ShipCount = 5;
    isFinished = false;

    for (int i = 0; i < 10; i++) {
        p1Board[i] = new char[10];
        p2Board[i] = new char[10];
        for (int j = 0; j < 10; j++) {
            p1Board[i][j] = emptySpace;
            p2Board[i][j] = emptySpace;
        }
    }

    // Placing the seed here ensures that both board are random.
    srand(time(NULL));
    placeShips(p1Board, p1Ships, emptySpace);
    placeShips(p2Board, p2Ships, emptySpace);
}

// Places the ships randomly on the board.
void Battleship::placeShips(char** &board, vector<Ship> &ships, char emptySpace) {
    for (int i = 5; i > 0; i--) {
        int x = rand() % 10;
        int y = rand() % 10;

        // If an existing position is selected.
        if (board[y][x] != emptySpace) {
            i++;
            continue;
        }

        // Set the ship type (using ships from Hasbro 2002 version)
        // and adds the ship's data to the vector.
        char shipType;
        int shipLength = i;
        Ship newShip;
        switch (i) {
            case 5:
                shipType = 'C';
                newShip = {shipType, "Carrier", shipLength};
                break;
            case 4:
                shipType = 'B';
                newShip = {shipType, "Battleship", shipLength};
                break;
            case 3:
                shipType = 'D';
                newShip = {shipType, "Destroyer", shipLength};
                break;
            case 2:
                shipType = 'S';
                shipLength = 3;
                newShip = {shipType, "Submarine", shipLength};
                break;
            case 1:
                shipType = 'P';
                shipLength = 2;
                newShip = {shipType, "Patrol Boat", shipLength};
                break;
        }
        ships.push_back(newShip);

        // Stores the possible placements in the co-ordinate.
        vector<direction> validDir = getDirections(x, y, emptySpace, shipLength, board);
        
        // If it's impossible to place the (whole) ship.
        if (validDir.size() == 0) {
            i++;
            continue;
        }

        // Randomly choose the possible direction.
        int dirIndex = rand() % validDir.size();
        direction placeDir = validDir[dirIndex];

        // Clear up the vector.
        validDir.clear();
        validDir.shrink_to_fit();

        switch (placeDir) {
            case UP:
                for (int j = 0; j < shipLength; j++)
                    board[y - j][x] = shipType;
                break;
            case DOWN:
                for (int j = 0; j < shipLength; j++)
                    board[y + j][x] = shipType;
                break;
            case LEFT:
                for (int j = 0; j < shipLength; j++)
                    board[y][x - j] = shipType;
                break;
            case RIGHT:
                for (int j = 0; j < shipLength; j++)
                    board[y][x + j] = shipType;
                break;
        }
    }
}

// Gets the valid placement directions for a ship.
vector<direction> Battleship::getDirections(int x, int y, char emptySpace, int shipLength, char** board) {
    vector<direction> validDir;
    bool upValid = true;
    bool downValid = true;
    bool leftValid = true;
    bool rightValid = true;

    for (int j = 0; j < shipLength; j++) {
        // UP
        if ((y - shipLength < 0) || board[y - j][x] != emptySpace)
            upValid = false;

        // DOWN
        if ((y + shipLength > 9) || board[y + j][x] != emptySpace)
            downValid = false;

        // LEFT
        if ((x - shipLength < 0) || board[y][x - j] != emptySpace)
            leftValid = false;

        // RIGHT
        if ((x + shipLength > 9) || board[y][x + j] != emptySpace)
            rightValid = false;

        // Exit early if placement is impossible.
        if (!upValid && !downValid && !leftValid && !rightValid)
            break;
    }

    if (upValid)
        validDir.push_back(UP);
    if (downValid)
        validDir.push_back(DOWN);
    if (leftValid)
        validDir.push_back(LEFT);
    if (rightValid)
        validDir.push_back(RIGHT);

    return validDir;
}

// Takes the player's co-ordinates to perform their turn.
void Battleship::shoot(char charX, int y) {
    int x = charX - 'A';
    y--; // Decrement y for index use.

    bool shipHit = false;
    char shipType;

    // Check what was hit.
    switch (p2Board[y][x]) {
        // If a ship is hit.
        case 'C':
        case 'B':
        case 'D':
        case 'S':
        case 'P':
            shipHit = true;
            shipType = p2Board[y][x];
            p2Board[y][x] = 'X';
            cout << "You've hit an enemy ship." << endl;
            break;
        case '~':
            p2Board[y][x] = 'O';
            cout << "Nothing was hit." << endl;
            break;
        default:
            // The position was already hit.
            throw logic_error("You've hit this position already.");
    }

    // If a ship was hit.
    if (shipHit) {
        // Check which ship it was.
        for (int j = 0; j < 5; j++) {
            if (p2Ships[j].codeName == shipType) {
                p2Ships[j].health--;
                // If the resulting hit sunk the ship.
                if (p2Ships[j].health == 0) {
                    cout << "You've sunk the enemy's " << p2Ships[j].fullName << '.' << endl;
                    p2ShipCount--;
                }
                break;
            }
        }
    }

    // If all the ships have sunk.
    if (p2ShipCount == 0) {
        isFinished = true;
        this->showBoard();
        cout << "Congratulations! You have sunk all the enemy ships." << endl;
        return;
    }

    // Perform opponent's turn.
}

// Checks if the game is over or not.
bool Battleship::isGameFinished() {
    return isFinished;
}

// Show the current contents of the boards.
void Battleship::showBoard() {
    cout << endl;
    cout << "         Your Board       |" << "      A B C D E F G H I J" << endl;
    cout << "   ---------------------  |" << "    ---------------------" << endl;
    for (int i = 0; i < 10; i++) {
        // Print line of the first board.
        for (int j = 0; j < 10; j++) {
            if (j == 0 && i == 9) {
                cout << i + 1 << " | " << p1Board[i][j] << ' ';
            } else if (j == 0) {
                cout << ' ' << i + 1 << " | " << p1Board[i][j] << ' ';
            } else {
                cout << p1Board[i][j] << ' ';
            }
        }

        // Print line of the second board.
        for (int j = 0; j < 10; j++) {
            // Hide the opponents ships.
            switch (p2Board[i][j]) {
                case 'C':
                case 'B':
                case 'D':
                case 'S':
                case 'P':
                    if (j == 0 && i == 9) {
                        cout << " | " << i + 1 << " | ~ ";
                    } else if (j == 0) {
                        cout << " |  " << i + 1 << " | ~ ";
                    } else {
                        cout << "~ ";
                    }
                    break;
               default:
                    if (j == 0 && i == 9) {
                        cout << " | " << i + 1 << " | " << p2Board[i][j] << ' ';
                    } else if (j == 0) {
                        cout << " |  " << i + 1 << " | " << p2Board[i][j] << ' ';
                    } else {
                        cout << p2Board[i][j] << ' ';
                    }
            }
        }
        cout << endl;
    }
}