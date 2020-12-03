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

    // Set the ships for player one.
    if (loadP1ShipFile) {
        getShipsFromFile("P1 Board.txt", p1Board);
        setShipData(p1Ships);
    } else {
        placeShips(p1Board, p1Ships);
    }

    // Set ships for player two.
    if (loadP2ShipFile) {
        getShipsFromFile("P2 Board.txt", p2Board);
        setShipData(p2Ships);
    } else {
        placeShips(p2Board, p2Ships);
    }
}

// Reads the ships from the specified file.
void Battleship::getShipsFromFile(string fileName, char** &currBoard) {
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
            }
        }
        colNum = 0;
        rowNum++;
    }
    boardFile.close();
}

// Sets the information for each ship.
// Used when random ship placement isn't used.
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

// Places the ships randomly on the board and sets the data for each ship.
void Battleship::placeShips(char** &board, unordered_map<char, Ship> &ships) {
    for (int i = 5; i > 0; i--) {
        int x = rand() % 10;
        int y = rand() % 10;

        // If an existing position is selected.
        if (board[y][x] != emptySpace) {
            i++;
            continue;
        }

        // Set the ship type (using ships from Hasbro 2002 version)
        // and adds the ship's data to the map.
        char shipType;
        int shipLength = i;
        Ship newShip;
        switch (i) {
            case 5:
                shipType = 'C';
                newShip = {"Carrier", shipLength, shipLength};
                break;
            case 4:
                shipType = 'B';
                newShip = {"Battleship", shipLength, shipLength};
                break;
            case 3:
                shipType = 'D';
                newShip = {"Destroyer", shipLength, shipLength};
                break;
            case 2:
                shipType = 'S';
                shipLength = 3;
                newShip = {"Submarine", shipLength, shipLength};
                break;
            case 1:
                shipType = 'P';
                shipLength = 2;
                newShip = {"Patrol Boat", shipLength, shipLength};
                break;
        }
        ships[shipType] = newShip;

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
    char** &currBoard = (currPlayer == 1) ? p2Board : p1Board;
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
        // Get possible moves for the first ship (could be any
        // because it's from an unordered map, but it doesn't matter).
        string shipKey = cpuMoves.begin()->first;
        queue<Coordinate> &shipMoves = cpuMoves[shipKey];
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
    string shipKey = thatShip.getName();

    // If the key (ship) doesn't exist.
    if (shipPosFound.find(shipKey) == shipPosFound.end()) {
        // Create hit position queue.
        queue<Coordinate> hitPos;
        hitPos.push(Coordinate(x, y));
        shipPosFound[shipKey] = hitPos;

        // NOTE: if a shot is missed (after this turn) when trying to find
        // the ship position, backtracking won't occur.
        prevShipHit.setName("");

        // Create possible moves queue.
        queue<Coordinate> possibleMoves;
        
        // Up, down, left, right (respectively).
        // Adds positions around where the ship was hit (don't push positions already hit).
        // We don't know where the ship is positioned at this point.
        if (y > 0 && !isPosHit(x, y - 1))
            possibleMoves.push(Coordinate(x, y - 1));
        if (y < 9  && !isPosHit(x, y + 1)) 
            possibleMoves.push(Coordinate(x, y + 1));
        if (x > 0  && !isPosHit(x - 1, y))
            possibleMoves.push(Coordinate(x - 1, y));
        if (x < 9  && !isPosHit(x + 2, y))
            possibleMoves.push(Coordinate(x + 1, y));
        cpuMoves[shipKey] = possibleMoves;

    // Otherwise, push the coordinates into the existing queues.
    } else {
        //Adjust cpuMoves.
        queue<Coordinate> &currMoves = cpuMoves[shipKey];

        // If the ship was hit once previously, then remove the blank moves (for this ship).
        if (shipPosFound[shipKey].size() == 1)
            currMoves = {};

        Coordinate prevShipMove = shipPosFound[shipKey].back();
        Coordinate currShipMove(x, y);
        shipPosFound[shipKey].push(currShipMove);

        prevShipHit = thatShip; // Used if backtracking is needed.

        // Get the direction in respect with the previously hit position.
        Direction dir = getDirection(currShipMove, prevShipMove);

        switch(dir) {
            case UP:
                // Continue the direction if it's still in bounds and 
                // if the next position hasn't been hit.
                if (y > 0 && !isPosHit(x, y - 1)) {
                    currMoves.push(Coordinate(x, y - 1));
                } else {
                    // Otherwise, set moves to sink the ship.
                    setAltMoves(UP, currShipMove);
                }
                break;
            case DOWN:
                if (y < 9 && !isPosHit(x, y + 1)) {
                    currMoves.push(Coordinate(x, y + 1));
                } else {
                    setAltMoves(DOWN, currShipMove);
                }
                break;
            case LEFT:
                if (x > 0 && !isPosHit(x - 1, y)) {
                    currMoves.push(Coordinate(x - 1, y));
                } else {
                    setAltMoves(LEFT, currShipMove);
                }
                break;
            case RIGHT:
                if (x < 9 && !isPosHit(x + 1, y)) {
                    currMoves.push(Coordinate(x + 1, y));
                } else {
                    setAltMoves(RIGHT, currShipMove);
                }
                break;
        }
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

// Checks if the next position has been hit already.
// Used for the CPU.
bool Battleship::isPosHit(int x, int y) {
    switch (p1Board[y][x]) {
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
            // Hide P1's ships if it's a two player game.
            char currPiece = (numPlayers == 2) ? emptySpace : p1Board[i][j];
            switch (p1Board[i][j]) {
                case 'X':
                case 'O':
                    currPiece = p1Board[i][j];
                    break;
            }
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
            // Hide the opponents ships.
            char currPiece = emptySpace;
            switch (p2Board[i][j]) {
                case 'X':
                case 'O':
                    currPiece = p2Board[i][j];
                    break;
            }
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