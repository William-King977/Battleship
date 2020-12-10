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
    for (int i = 0; i < 10; i++) {
        delete[] p1Board[i];
        delete[] p2Board[i];
    }
    delete[] p1Board;
    delete[] p2Board;

    cpuMoves = {};
    shipPosFound = {};
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
    Ship prevShipHit; // Used for back tracking.

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

    // Set data for the ships.
    setShipData(p1Ships);
    setShipData(p2Ships);

    // Set the ship placements for player one.
    if (loadP1ShipFile) {
        getShipsFromFile("P1 Board.txt", p1Board);
    } else {
        placeShips(p1Board);
    }

    // Set ship placements for player two.
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
    if (!isBoardValid(currBoard)) {
        throw runtime_error(fileName + ", incorrect ship placements.");
    }
}

// Check if the board contents are valid (from a file).
bool Battleship::isBoardValid(char** board) {
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
                // The piece is invalid (not a ship/empty space).
                default:
                    return false;
            }

            char shipType = board[i][j];

            // Add a position queue for a ship (if it doesn't exist).
            if (visitedPos.find(shipType) == visitedPos.end()) {
                vector<Coordinate> newShipPos(1, Coordinate(j, i));
                visitedPos[shipType] = newShipPos;
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
                } else {
                    continue;
                }
            }

            // Check if the ship has been placed correctly.
            if (!isShipPlacementValid(board, visitedPos[shipType], shipType, shipLength)) {
                return false;
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
bool Battleship::isShipPlacementValid(char** board, vector<Coordinate> &shipPos, char shipType, int shipLength) {
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
            cout << "Nothing was hit." << endl;
            break;
        default:
            // The position was already hit.
            throw logic_error("You've hit this position already.");
    }

    // Holds the opponent's player number.
    string enemyPlayer = (currPlayer == 1) ? "Player 2" : "Player 1";

    // If a ship was hit.
    if (shipHit) {
        // Get the ship that was hit.
        Ship &thatShip = currShips[shipType];
        thatShip.setHealth(thatShip.getHealth() - 1);
        // Display a suitable message.
        if (numPlayers == 1) {
            cout << "You've hit the enemy's " << thatShip.getName() << '.' << endl;
        } else {
            cout << "You've hit " << enemyPlayer << "'s " << thatShip.getName() << '.' << endl;
        }

        // If the resulting hit sunk the ship.
        if (thatShip.getHealth() == 0) {
            currShipCount--;
            // Display a suitable message.
            if (numPlayers == 1) {
                cout << "You've sunk the enemy's " << thatShip.getName() << '.' << endl;
            } else {
                cout << "You've sunk " << enemyPlayer << "'s " << thatShip.getName() << '.' << endl;
            }
        }
    }

    // Show the number of ships remaining.
    string playerType = (numPlayers == 1) ? "CPU" : enemyPlayer;
    cout << "Ships Remaining (" << playerType << "): " << currShipCount << endl;

    // If all the ships have sunk.
    if (currShipCount == 0) {
        isFinished = true;
        this->showBoard();
        // Display a suitable message.
        if (numPlayers == 1) {
            cout << "Congratulations! You have sunk all the enemy ships." << endl;
        } else {
            cout << "Congratulations! You have sunk all of " << enemyPlayer << "'s ships." << endl;
            cout << "Player " << currPlayer << " wins!" << endl;
        }
        return;
    }

    if (numPlayers == 1) {
        // Perform the CPU's turn.
        cout << endl << "---------------------CPU's Turn---------------------" << endl;
        this->cpuShoot();
    } else {
        // Change players if it's a two player game.
        currPlayer = (currPlayer == 1) ? 2 : 1;
    }
}

// Performs the CPU's turn.
void Battleship::cpuShoot() {
    int x;
    int y;
    
    if (!cpuMoves.empty()) {
        // Get possible moves for a damaged, but unsunk ship.
        string shipKey = cpuMoves.begin()->first;
        queue<Coordinate> &shipMoves = cpuMoves[shipKey];

        // If there are no moves, then push the moves to sink it.
        if (shipMoves.size() == 0) {
            // setAltMoves() needs this.
            prevShipHit = p1Ships[shipKey[0]];
            Coordinate first = shipPosFound[shipKey].front();
            Coordinate last = shipPosFound[shipKey].back();
            // Direction of the ship being shot before going on another ship.
            setAltMoves(getDirection(last, first), last);
            cpuShoot();
            return;
        }

        x = shipMoves.front().getX();
        y = shipMoves.front().getY();
        shipMoves.pop();
    } else {
        x = rand() % 10;
        y = rand() % 10;
    }
    
    bool shipHit = false;
    char shipType;

    // Check what was hit.
    switch (p1Board[y][x]) {
        // If a ship is hit.
        case 'C':
        case 'B':
        case 'D':
        case 'S':
        case 'P':
            shipHit = true;
            shipType = p1Board[y][x];
            p1Board[y][x] = 'X';
            break;
        case emptySpace:
            p1Board[y][x] = 'O';
            cout << "Nothing was hit." << endl;
            // If there is a ship that has been hit (but not sunk),
            // then push the remaining moves to sink it.
            if ((prevShipHit.getName()).length() > 0) {
                backTrackShot(x, y);
            }
            break;
        default:
            // The position was already hit.
            // Recurse the method until a different position is chosen.
            cpuShoot();
            return;
    }

    // Show co-ordinates chosen.
    cout << "Co-ordinates: " << char(x + 'A') << y + 1 << endl;

    // If a ship was hit.
    if (shipHit) {
        // Get the ship that was hit.
        Ship &thatShip = p1Ships[shipType];
        thatShip.setHealth(thatShip.getHealth() - 1);
        cout << "Your " << thatShip.getName() << " has been hit." << endl;

        // If the resulting hit sunk the ship.
        if (thatShip.getHealth() == 0) {
            cout << "Your " << thatShip.getName() << " has sunk!" << endl;
            p1ShipCount--;
            // Remove ship from the maps.
            shipPosFound.erase(thatShip.getName());
            cpuMoves.erase(thatShip.getName());
            // Sets the next previous ship to sink.
            setPrevShip();
        // Only add moves if the ship has not sunk.
        } else {
            setCpuMoves(x, y, thatShip);
        }
    }

    // Show the number of ships remaining.
    cout << "Ships Remaining (Player 1): " << p1ShipCount << endl;

    // If all the ships have sunk.
    if (p1ShipCount == 0) {
        isFinished = true;
        this->showBoard();
        cout << "All your ships have sunk! The CPU wins." << endl;
    }
}

// Sets the possible moves for the CPU after a ship is hit.
void Battleship::setCpuMoves(int x, int y, Ship thatShip) {
    // If the key (ship) doesn't exist, try and find the ship's direction.
    if (shipPosFound.find(thatShip.getName()) == shipPosFound.end()) {
        findShip(x, y, thatShip);
    // Otherwise, push the coordinates into the existing queues (to sink the ship).
    } else {
        sinkShip(x, y, thatShip);
    }
}

// Pushes in moves that are used to find a ship's direction.
void Battleship::findShip(int x, int y, Ship thatShip) {
    // Create hit position queue (positions hit on the ship).
    queue<Coordinate> shipHitPos;
    Coordinate newPos(x, y);
    shipHitPos.push(newPos);
    shipPosFound[thatShip.getName()] = shipHitPos;

    // NOTE: if a shot is missed (after this turn) when trying to find
    // the ship's position, backtracking won't occur.
    prevShipHit.setName("");

    // Create possible moves queue (for cpuMoves).
    queue<Coordinate> possibleMoves;
        
    // These positions will be used as temperary boundaries.
    char tempUpChar = '\0';
    char tempLeftChar = '\0';

    // Up, down, left, right (respectively).
    // Adds positions around where the ship was hit.
    // We don't know where the ship is positioned at this point.
    if (y > 0 && !isPosHit(p1Board[y - 1][x]) && canShipExist(thatShip.getLength(), newPos, UP)) {
        // Temporarily mark it to adjust the 'boundaries' for the opposite direction.
        tempUpChar = p1Board[y - 1][x];
        p1Board[y - 1][x] = 'O';
        possibleMoves.push(Coordinate(x, y - 1));
    }
    if (y < 9  && !isPosHit(p1Board[y + 1][x]) && canShipExist(thatShip.getLength(), newPos, DOWN)) {
        possibleMoves.push(Coordinate(x, y + 1));
    }
    if (x > 0  && !isPosHit(p1Board[y][x - 1]) && canShipExist(thatShip.getLength(), newPos, LEFT)) {
        tempLeftChar = p1Board[y][x - 1];
        p1Board[y][x - 1] = 'O';
        possibleMoves.push(Coordinate(x - 1, y));
    }
    if (x < 9  && !isPosHit(p1Board[y][x + 1]) && canShipExist(thatShip.getLength(), newPos, RIGHT)) {
        possibleMoves.push(Coordinate(x + 1, y));
    }

    // Put back the characters and allocate the moves queue.
    if (tempUpChar != '\0') {
        p1Board[y - 1][x] = tempUpChar;
    }
    if (tempLeftChar != '\0') {
        p1Board[y][x - 1] = tempLeftChar;
    }
    cpuMoves[thatShip.getName()] = possibleMoves;
}

// Push in moves that are used to sink a ship.
void Battleship::sinkShip(int x, int y, Ship thatShip) {
    //Adjust cpuMoves.
    queue<Coordinate> &currMoves = cpuMoves[thatShip.getName()];

    // If the ship was hit once previously, then remove the blank moves (for this ship).
    if (shipPosFound[thatShip.getName()].size() == 1)
        currMoves = {};

    Coordinate prevShipMove = shipPosFound[thatShip.getName()].back();
    Coordinate currShipMove(x, y);
    shipPosFound[thatShip.getName()].push(currShipMove);

    prevShipHit = thatShip; // Used if backtracking is needed.

    // Get the direction in respect with the previously hit position.
    Direction dir = getDirection(currShipMove, prevShipMove);

    switch(dir) {
        case UP:
            // Continue the direction if it's still in bounds and 
            // if the next position hasn't been hit.
            if (y > 0 && !isPosHit(p1Board[y - 1][x])) {
                currMoves.push(Coordinate(x, y - 1));
            } else {
                // Otherwise, set moves to sink the ship.
                setAltMoves(UP, currShipMove);
            }
            break;
        case DOWN:
            if (y < 9 && !isPosHit(p1Board[y + 1][x])) {
                currMoves.push(Coordinate(x, y + 1));
            } else {
                setAltMoves(DOWN, currShipMove);
            }
            break;
        case LEFT:
            if (x > 0 && !isPosHit(p1Board[y][x - 1])) {
                currMoves.push(Coordinate(x - 1, y));
            } else {
                setAltMoves(LEFT, currShipMove);
            }
            break;
        case RIGHT:
            if (x < 9 && !isPosHit(p1Board[y][x + 1])) {
                currMoves.push(Coordinate(x + 1, y));
            } else {
                setAltMoves(RIGHT, currShipMove);
            }
            break;
    }
}

// Adjusts the possible moves when missing a shot while trying to sink a ship.
void Battleship::backTrackShot(int x, int y) {
    string shipKey = prevShipHit.getName();
    Coordinate prevShipMove = shipPosFound[shipKey].back();

    // Get direction, then push in remaining moves to sink the ship.
    Direction dir = getDirection(Coordinate(x, y), prevShipMove);
    setAltMoves(dir, prevShipMove);
}

// Sets the moves to sink a discovered ship.
void Battleship::setAltMoves(Direction dir, Coordinate prevShipMove) {
    string shipKey = prevShipHit.getName();
    queue<Coordinate> &currMoves = cpuMoves[shipKey];

    int x = prevShipMove.getX();
    int y = prevShipMove.getY();

    // Used to determine new moves.
    int timesHit = shipPosFound[shipKey].size(); // Could use length - health...

    // Only consider ships with length 3 or more (A patrol boat would've been sunk already).
    switch (dir) {
        // If you went Up, go back Down.
        case UP:
            // Based on the ship's remaining health (it has been hit at least TWICE).
            switch (prevShipHit.getHealth()) {
                case 3:
                    currMoves.push(Coordinate(x, y + (timesHit + 2)));
                case 2:
                    currMoves.push(Coordinate(x, y + (timesHit + 1)));
                case 1:
                    currMoves.push(Coordinate(x, y + timesHit));
            }
            break;
        // If you went Down, go back Up.
        case DOWN:
            switch (prevShipHit.getHealth()) {
                case 3:
                    currMoves.push(Coordinate(x, y - (timesHit + 2)));
                case 2:
                    currMoves.push(Coordinate(x, y - (timesHit + 1)));
                case 1:
                    currMoves.push(Coordinate(x, y - timesHit));
            }
            break;
        // If you went Left, go Right.  
        case LEFT:
            switch (prevShipHit.getHealth()) {
                case 3:
                    currMoves.push(Coordinate(x + (timesHit + 2), y));
                case 2:
                    currMoves.push(Coordinate(x + (timesHit + 1), y));
                case 1:
                    currMoves.push(Coordinate(x + timesHit, y));
            }
            break;
        // If you went Right, go Left.
        case RIGHT:
            switch (prevShipHit.getHealth()) {
                case 3:
                    currMoves.push(Coordinate(x - (timesHit + 2), y));
                case 2:
                    currMoves.push(Coordinate(x - (timesHit + 1), y));
                case 1:
                    currMoves.push(Coordinate(x - timesHit, y));
            }
            break;
    }
}

// Sets the new previous ship, once a ship has sunk.
void Battleship::setPrevShip() {
    // Fetch a ship from shipPosFound (doesn't matter which).
    if (!shipPosFound.empty()) {
        string shipName = shipPosFound.begin()->first;
        Ship currShip = p1Ships[shipName[0]];
        prevShipHit = currShip;

        // Push moves to sink the unsunk ship.
        if (cpuMoves[shipName].empty()) {
            Coordinate prevMove = shipPosFound[shipName].back();
            Coordinate firstMove = shipPosFound[shipName].front();
            Direction dir = getDirection(prevMove, firstMove);
            setAltMoves(dir, prevMove);
        }
    } else {
        // Indicate that there are no ships to sink (for now).
        prevShipHit.setName("");
    }
}

// Returns a direction based on two coordinates of a ship.
Direction Battleship::getDirection(Coordinate first, Coordinate last) {
    int x = first.getX();
    int y = first.getY();
    int prevX = last.getX();
    int prevY = last.getY();
    Direction dir;

    // Check and set direction.
    if (y < prevY) {
        dir = UP;
    } else if (y > prevY) {
        dir = DOWN;
    } else if (x < prevX) {
        dir = LEFT;
    } else if (x > prevX) {
        dir = RIGHT;
    }
    return dir;
}

// Checks if a ship's existence is possible in a given direction.
bool Battleship::canShipExist(int shipLength, Coordinate currPos, Direction dir) {
    bool upperInBounds = true;
    bool lowerInBounds = true;
    int upperBound;
    int lowerBound;
    
    switch (dir) {
        case UP:
        case DOWN:
            upperBound = currPos.getY();
            lowerBound = currPos.getY();
            for (int i = 1; i < shipLength; i++) {
                int upPos = currPos.getY() - i;
                int downPos = currPos.getY() + i;
                // UP
                if (upPos >= 0 && !isPosHit(p1Board[upPos][currPos.getX()]) && lowerInBounds) {
                    lowerBound--;
                } else {
                    lowerInBounds = false;
                }
                
                // DOWN
                if (downPos <= 9 && !isPosHit(p1Board[downPos][currPos.getX()]) && upperInBounds) {
                    upperBound++;
                } else {
                    upperInBounds = false;
                }
            }
            break;
        case LEFT:
        case RIGHT:
            upperBound = currPos.getX();
            lowerBound = currPos.getX();
            for (int i = 1; i < shipLength; i++) {
                int leftPos = currPos.getX() - i;
                int rightPos = currPos.getX() + i;
                // LEFT
                if (leftPos >= 0 && !isPosHit(p1Board[currPos.getY()][leftPos]) && lowerInBounds) {
                    lowerBound--;
                } else {
                    lowerInBounds = false;
                }
                
                // RIGHT
                if (rightPos <= 9 && !isPosHit(p1Board[currPos.getY()][rightPos]) && upperInBounds) {
                    upperBound++;
                } else {
                    upperInBounds = false;
                }
            }
            break;
    }
    int innerBound = upperBound - lowerBound;
    bool placementPossible = (innerBound + 1) >= shipLength ? true : false;
    return placementPossible;
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
        // Print line of the first board.
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

        // Print line of the second board.
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