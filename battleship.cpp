#include "battleship.hpp"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <exception>
#include <cstdlib>
#include <ctime>
using namespace std;

Battleship::Battleship() {
    // cout << "Battleship object made." << endl;
}

// Deconstructor deletes/clears certain data structures.
Battleship::~Battleship() {
    // cout << "Battleship object destroyed." << endl;
    for (int i = 0; i < 10; i++) {
        delete[] p1Board[i];
        delete[] p2Board[i];
    }
    delete[] p1Board;
    delete[] p2Board;

    p1Ships = {};
    p2Ships = {};
}

// Initialises the game components and fills the board.
void Battleship::startGame(int numPlayers, bool loadP1ShipFile, bool loadP2ShipFile) {
    p1Board = new char* [10];
    p2Board = new char* [10];

    this->numPlayers = numPlayers;
    currPlayer = 1; // Whose turn it is.
    p1ShipCount = 5;
    p2ShipCount = 5;
    isFinished = false;
    p1Win = false;
    p2Win = false;

    for (int i = 0; i < 10; i++) {
        p1Board[i] = new char[10];
        p2Board[i] = new char[10];
        for (int j = 0; j < 10; j++) {
            p1Board[i][j] = emptySpace;
            p2Board[i][j] = emptySpace;
        }
    }

    // Placing the seed here ensures that both boards are random.
    srand(time(NULL));

    // Set data for the ships.
    setShipData(p1Ships);
    setShipData(p2Ships);

    // Set the ship placements for Player 1.
    if (loadP1ShipFile) {
        getShipsFromFile("P1 Board.txt", p1Board);
    } else {
        placeShips(p1Board);
    }

    // Set ship placements for Player 2.
    if (loadP2ShipFile) {
        getShipsFromFile("P2 Board.txt", p2Board);
    } else {
        placeShips(p2Board);
    }
}

// Reads the ships from the specified file.
void Battleship::getShipsFromFile(string fileName, char** currBoard) {
    const string boardDir = "boards/" + fileName;
    ifstream boardFile(boardDir);

    // Checks if it exists.
    struct stat buffer;
    if (stat(boardDir.c_str(), &buffer)) {
        throw runtime_error("The file '" + fileName + "' cannot be found.");
    }

    // If it has restricted access.
    if (!boardFile.is_open()) {
        throw runtime_error("No rights to access the file, " + fileName);
    }

    string row;
    int rowNum = 0;
    int colNum = 0;
    
    while (getline(boardFile, row)) {
        // Insert pieces from each row.
        for (int i = 0; i < row.length(); i++) {
            // Each piece is seperated by a whitespace.
            switch (row[i]) {
                case 'C':
                case 'B':
                case 'D':
                case 'S':
                case 'P':
                    currBoard[rowNum][colNum] = row[i];
                    colNum++;
                    break;
                case emptySpace:
                    colNum++;
                    break;
                case ' ':
                    break;
                // Invalid piece.
                default:
                    boardFile.close();
                    throw runtime_error(fileName + ", invalid piece in row " + to_string(rowNum + 1) + 
                                    ", column " + to_string(colNum + 1) + '.');
            }

            // Too many columns (row/colNum should at most 10).
            if (colNum > 10) {
                boardFile.close();
                throw runtime_error(fileName + ", too many columns (" + to_string(colNum) + 
                                    " columns in row " + to_string(rowNum + 1) + ").");
            }
        }
        // Not enough columns.
        if (colNum < 10) {
            boardFile.close();
            throw runtime_error(fileName + ", not enough columns (" + to_string(colNum) + 
                                " columns in row " + to_string(rowNum + 1) + ").");
        }

        colNum = 0;
        rowNum++;

        // Too many rows.
        if (rowNum > 10) {
            boardFile.close();
            throw runtime_error(fileName + ", too many rows (" + to_string(rowNum) + " rows).");
        }
    }
    boardFile.close();

    // Not enough rows.
    if (rowNum < 10) {
        throw runtime_error(fileName + ", not enough rows (" + to_string(rowNum) + " rows).");
    }

    // Check the ship placements.
    if (!isShipPlacementValid(currBoard)) {
        throw runtime_error(fileName + ", incorrect ship placements.");
    }
}

// Check if the board contents are valid (from a file).
bool Battleship::isShipPlacementValid(char** board) {
    // Holds previously visited positions.
    unordered_map<char, vector<Coordinate>> visitedPos;

    // Go through the board.
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int shipLength;
            switch (board[i][j]) {
                // Set the ship length.
                case 'C':
                    shipLength = 5;
                    break;
                case 'B':
                    shipLength = 4;
                    break;
                case 'D':
                case 'S':
                    shipLength = 3;
                    break;
                case 'P':
                    shipLength = 2;
                    break;
                // Ignore empty spaces.
                case emptySpace:
                    continue;
                    break;
                // The piece is invalid (it shouldn't reach here...).
                default:
                    return false;
            }

            char shipType = board[i][j];

            // Add a position queue for a ship (if it doesn't exist).
            if (visitedPos.find(shipType) == visitedPos.end()) {
                vector<Coordinate> newShipPos(1, Coordinate(j, i));
                visitedPos[shipType] = newShipPos;
                // Check if the ship has been placed correctly.
                // NOTE: the remainder of the ship's positions will be added to the vector.
                if (!isShipValid(board, visitedPos[shipType], shipType, shipLength)) {
                    return false;
                }
            } else {
                bool isShipSame = false;
                vector<Coordinate> existShipPos = visitedPos[shipType];
                // Check if it's a piece of an existing ship.
                for (Coordinate pos : existShipPos) {
                    if ((pos.getY() == i) && (pos.getX() == j)) {
                        isShipSame = true;
                        break;
                    }
                }

                // If it isn't, then the user has placed two of the same ship...
                if (!isShipSame) {
                    return false;
                }
            }
        }
    }

    // If there are not enough ships.
    if (visitedPos.size() != 5) {
        return false;
    }

    // The whole board is valid.
    return true;
}

// Checks if a placement for a ship is valid.
bool Battleship::isShipValid(char** board, vector<Coordinate> &shipPos, char shipType, int shipLength) {
    Coordinate foundPos = shipPos.front();
    int currSize = 1;

    // Check if it's placed vertically.
    for (int i = 1; i < shipLength; i++) {
        // Check downwards (the board is checked left to right, top to bottom).
        int downPos = foundPos.getY() + i;
        if (downPos <= 9 && board[downPos][foundPos.getX()] == shipType) {
            shipPos.push_back(Coordinate(foundPos.getX(), downPos));
            currSize++;
        } else {
            break;
        }
    }
    if (currSize == shipLength) {
        return true;
    }

    // Reset currSize.
    currSize = 1;

    // Check if it's placed horizontally.
    for (int i = 1; i < shipLength; i++) {
        // Check to the right.
        int rightPos = foundPos.getX() + i;
        if (rightPos <= 9 && board[foundPos.getY()][rightPos] == shipType) {
            shipPos.push_back(Coordinate(rightPos, foundPos.getY()));
            currSize++;
        } else {
            break;
        }
    }
    if (currSize == shipLength) {
        return true;
    }

    // Incorrect ship size.
    return false;
}

// Sets the information for each ship.
void Battleship::setShipData(unordered_map<char, Ship> &ships) {
    for (int i = 2; i <= 5; i++) {
        switch (i) {
            case 5:
                ships['C'] = {"Carrier", i, i};
                break;
            case 4:
                ships['B'] = {"Battleship", i, i};
                break;
            case 3:
                ships['D'] = {"Destroyer", i, i};
                ships['S'] = {"Submarine", i, i};
                break;
            case 2:
                ships['P'] = {"Patrol Boat", i, i};
                break;
        }
    }
}

// Places the ships randomly on the board.
void Battleship::placeShips(char** board) {
    // Place the bigger ships first.
    for (int i = 5; i > 0; i--) {
        int x = rand() % 10;
        int y = rand() % 10;

        // If an existing position is selected.
        if (board[y][x] != emptySpace) {
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
        vector<Direction> validDir = getValidDirections(x, y, shipLength, board);
        
        // If it's impossible to place the (whole) ship.
        if (validDir.size() == 0) {
            i++;
            continue;
        }

        // Randomly choose the possible direction.
        int dirIndex = rand() % validDir.size();
        Direction placeDir = validDir[dirIndex];

        // Clear up the vector.
        validDir.clear();
        validDir.shrink_to_fit();

        // Place the ships on the board.
        switch (placeDir) {
            case UP:
                for (int j = 0; j < shipLength; j++) {
                    board[y - j][x] = shipType;
                }
                break;
            case DOWN:
                for (int j = 0; j < shipLength; j++) {
                    board[y + j][x] = shipType;
                }
                break;
            case LEFT:
                for (int j = 0; j < shipLength; j++) {
                    board[y][x - j] = shipType;
                }
                break;
            case RIGHT:
                for (int j = 0; j < shipLength; j++) {
                    board[y][x + j] = shipType;
                }
                break;
        }
    }
}

// Gets the valid placement directions for a ship.
vector<Direction> Battleship::getValidDirections(int x, int y, int shipLength, char** board) {
    vector<Direction> validDir;
    bool upValid = true;
    bool downValid = true;
    bool leftValid = true;
    bool rightValid = true;

    // Try and place the ship in each direction.
    for (int j = 0; j < shipLength; j++) {
        // UP
        if ((y - shipLength < 0) || board[y - j][x] != emptySpace) {
            upValid = false;
        }
        // DOWN
        if ((y + shipLength > 9) || board[y + j][x] != emptySpace) {
            downValid = false;
        }
        // LEFT
        if ((x - shipLength < 0) || board[y][x - j] != emptySpace) {
            leftValid = false;
        }
        // RIGHT
        if ((x + shipLength > 9) || board[y][x + j] != emptySpace) {
            rightValid = false;
        }
        // Exit early if placement is impossible.
        if (!upValid && !downValid && !leftValid && !rightValid) {
            break;
        }
    }

    if (upValid) {
        validDir.push_back(UP);
    }
    if (downValid) {
        validDir.push_back(DOWN);
    }
    if (leftValid) {
        validDir.push_back(LEFT);
    }
    if (rightValid) {
        validDir.push_back(RIGHT);
    }

    return validDir;
}

// Takes the player's co-ordinates to perform their turn.
void Battleship::shoot(char charX, int y) {
    // Set the current board, ships and ship count.
    char** currBoard = (currPlayer == 1) ? p2Board : p1Board;
    unordered_map<char, Ship> &currShips = (currPlayer == 1) ? p2Ships : p1Ships;
    int &currShipCount = (currPlayer == 1) ? p2ShipCount : p1ShipCount;

    int x = charX - 'A';
    y--; // Decrement y for index use.

    bool shipHit = false;
    char shipType;

    // Check what was hit.
    switch (currBoard[y][x]) {
        // If a ship is hit.
        case 'C':
        case 'B':
        case 'D':
        case 'S':
        case 'P':
            shipHit = true;
            shipType = currBoard[y][x];
            currBoard[y][x] = 'X';
            break;
        case emptySpace:
            currBoard[y][x] = 'O';
            cout << "Miss." << endl;
            break;
        default:
            // The position was already hit.
            throw logic_error("You've hit this position already.");
    }

    // If a ship was hit.
    if (shipHit) {
        // Get the ship that was hit.
        Ship &thatShip = currShips[shipType];
        thatShip.setHealth(thatShip.getHealth() - 1);

        // If the resulting hit sunk the ship.
        if (thatShip.getHealth() == 0) {
            currShipCount--;
            // Display a suitable message.
            cout << "Hit and sunk. " << thatShip.getName() << '.' << endl;
        } else {
            cout << "Hit. " << thatShip.getName() << '.' << endl;
        }
    }

    // Show the number of ships sunk.
    cout << "Ships Sunk: " << (5 - currShipCount) << endl;

    // If all the opponent's ships have sunk.
    if (currShipCount == 0) {
        // Change the win status of the player.
        switch (currPlayer) {
            case 2:
                p2Win = true;
                break;
            case 1:
                p1Win = true;
                break;
        }
    }

    // Change players if it's a two player game.
    if (numPlayers == 2) {
        currPlayer = (currPlayer == 1) ? 2 : 1;
    }
}

// Checks if a position has been hit.
bool Battleship::isPosHit(char boardPiece) {
    switch (boardPiece) {
        case 'X':
        case 'O':
            return true;
        default:
            return false;
    }
}

// Show the current contents of the boards.
void Battleship::showBoard() {
    string enemyName = (numPlayers == 1) ? " CPU  " : " P2   ";

    cout << endl << "P1   A B C D E F G H I J  |" << enemyName << "A B C D E F G H I J" << endl;
    cout << "   ---------------------  |" << "    ---------------------" << endl;
    for (int i = 0; i < 10; i++) {
        // Print the line of the first board.
        for (int j = 0; j < 10; j++) {
            // Show P1's ships if it's hit or if it's a single player game.
            char currPiece = ((numPlayers == 1) || isPosHit(p1Board[i][j])) ? p1Board[i][j] : emptySpace;
            
            if (j == 0 && i == 9) {
                cout << i + 1 << " | " << currPiece << ' ';
            } else if (j == 0) {
                cout << ' ' << i + 1 << " | " << currPiece << ' ';
            } else {
                cout << currPiece << ' ';
            }
        }

        // Print the line of the second board.
        for (int j = 0; j < 10; j++) {
            // Hide the opponents ships if they're not hit.
            char currPiece = isPosHit(p2Board[i][j]) ? p2Board[i][j] : emptySpace;
            
            if (j == 0 && i == 9) {
                cout << " | " << i + 1 << " | " << currPiece << ' ';
            } else if (j == 0) {
                cout << " |  " << i + 1 << " | " << currPiece << ' ';
            } else {
                cout << currPiece << ' ';
            }
        }
        cout << endl;
    }
}