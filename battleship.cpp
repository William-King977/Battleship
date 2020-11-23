#include "battleship.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>

Battleship::Battleship() {
    // cout << "Battleship object made." << endl;
}

Battleship::~Battleship() {
    for (int i = 0; i < 10; i++) {
        delete[] p1Board[i];
    }
    delete[] p1Board;
    // cout << "Battleship object deconstructed";
}

// Initialises and fills the board.
void Battleship::startGame() {
    char emptySpace = '~';
    p1Board = new char* [10];

    for (int i = 0; i < 10; i++) {
        p1Board[i] = new char[10];
        for (int j = 0; j < 10; j++) {
            p1Board[i][j] = emptySpace;
        }
    }
    this->placeShips(emptySpace);
}

// Places the ships randomly on the board.
void Battleship::placeShips(char emptySpace) {
    srand(time(NULL));

    for (int i = 5; i > 0; i--) {
        int x = rand() % 10;
        int y = rand() % 10;

        // If an existing position is selected.
        if (p1Board[y][x] != emptySpace) {
            i++;
            continue;
        }

        // Set the ship type (using ships from Hasbro 2002 version).
        char shipType;
        int shipLength = i;
        switch (i) {
            case 5:
                shipType = 'C';
                break;
            case 4:
                shipType = 'B';
                break;
            case 3:
                shipType = 'D';
                break;
            case 2:
                shipType = 'S';
                shipLength = 3;
                break;
            case 1:
                shipType = 'P';
                shipLength = 2;
                break;
        }

        // Stores the possible placements in the co-ordinate.
        vector<direction> validDir = getDirections(x, y, emptySpace, shipLength);
        
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
                    p1Board[y - j][x] = shipType;
                break;
            case DOWN:
                for (int j = 0; j < shipLength; j++)
                    p1Board[y + j][x] = shipType;
                break;
            case LEFT:
                for (int j = 0; j < shipLength; j++)
                    p1Board[y][x - j] = shipType;
                break;
            case RIGHT:
                for (int j = 0; j < shipLength; j++)
                    p1Board[y][x + j] = shipType;
                break;
        }
    }
}

// Gets the valid placement directions for a ship.
vector<direction> Battleship::getDirections(int x, int y, char emptySpace, int shipLength) {
    vector<direction> validDir;
    bool upValid = true;
    bool downValid = true;
    bool leftValid = true;
    bool rightValid = true;

    for (int j = 0; j < shipLength; j++) {
        // UP
        if ((y - shipLength < 0) || p1Board[y - j][x] != emptySpace)
            upValid = false;

        // DOWN
        if ((y + shipLength > 9) || p1Board[y + j][x] != emptySpace)
            downValid = false;

        // LEFT
        if ((x - shipLength < 0) || p1Board[y][x - j] != emptySpace)
            leftValid = false;

        // RIGHT
        if ((x + shipLength > 9) || p1Board[y][x + j] != emptySpace)
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

// Show the current contents of the board.
void Battleship::showBoard() {
    cout << "     A B C D E F G H I J" << endl;
    cout << "   ---------------------" << endl;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (j == 0 && i == 9) {
                cout << i + 1 << " | " << p1Board[i][j] << ' ';
            } else if (j == 0) {
                cout << ' ' << i + 1 << " | " << p1Board[i][j] << ' ';
            } else {
                cout << p1Board[i][j] << ' ';
            }
        }
        cout << endl;
    }
}