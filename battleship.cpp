#include "battleship.hpp"
#include <iostream>
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
    p1Ships.clear();
    p2Ships.clear();
}

// Initialises the game components and fills the board.
void Battleship::startGame() {
    p1Board = new char* [10];
    p2Board = new char* [10];

    p1ShipCount = 5;
    p2ShipCount = 5;
    isFinished = false;
    Ship prevShipHit = {"", 0, 0}; // Used for back tracking.

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
    placeShips(p1Board, p1Ships);
    placeShips(p2Board, p2Ships);
}

// Places the ships randomly on the board.
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
        vector<direction> validDir = getDirections(x, y, shipLength, board);
        
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
vector<direction> Battleship::getDirections(int x, int y, int shipLength, char** board) {
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
            break;
        case emptySpace:
            p2Board[y][x] = 'O';
            cout << "Nothing was hit." << endl;
            break;
        default:
            // The position was already hit.
            throw logic_error("You've hit this position already.");
    }

    // If a ship was hit.
    if (shipHit) {
        // Get the ship that was hit.
        Ship &thatShip = p2Ships[shipType];
        thatShip.health--;
        cout << "You've hit the enemy's " << thatShip.name << '.' << endl;

        // If the resulting hit sunk the ship.
        if (thatShip.health == 0) {
            cout << "You've sunk the enemy's " << thatShip.name << '.' << endl;
            p2ShipCount--;
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
    cout << endl << "---------------------CPU's Turn---------------------" << endl;
    this->enemyShoot();
}

// Performs the CPU's turn.
void Battleship::enemyShoot() {
    int x;
    int y;

    if (!cpuMoves.empty()) {
        // Get possible moves for the first ship (could be any
        // because it's from an unordered map, but it doesn't matter).
        string shipKey = cpuMoves.begin()->first;
        queue<Coordinate> &shipMoves = cpuMoves[shipKey];
        x = shipMoves.front().x;
        y = shipMoves.front().y;
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
            setPrevShip();
            if ((prevShipHit.name.length()) > 0)
                backTrackShot(x, y);
            break;
        default:
            // The position was already hit.
            // Recurse the method until a different position is chosen.
            enemyShoot();
            return;
    }

    // Show co-ordinates chosen.
    cout << "Co-ordinates: " << char(x + 'A') << y + 1 << endl;

    // If a ship was hit.
    if (shipHit) {
        // Get the ship that was hit.
        Ship &thatShip = p1Ships[shipType];
        thatShip.health--;
        cout << "Your " << thatShip.name << " has been hit." << endl;

        // If the resulting hit sunk the ship.
        if (thatShip.health == 0) {
            cout << "Your " << thatShip.name << " has sunk!" << endl;
            p1ShipCount--;
            // Remove ship from the maps.
            shipPosFound.erase(thatShip.name);
            cpuMoves.erase(thatShip.name);
        // Only add moves if the ship has not sunk.
        } else {
            setCpuMoves(x, y, thatShip);
        }
    }

    // If all the ships have sunk.
    if (p1ShipCount == 0) {
        isFinished = true;
        this->showBoard();
        cout << "All your ships have sunk! The CPU wins." << endl;
    }
}

// Sets the possible moves for the CPU after a ship is hit.
void Battleship::setCpuMoves(int x, int y, Ship thatShip) {
    string shipKey = thatShip.name;

    // If the key (ship) doesn't exist.
    if (shipPosFound.find(shipKey) == shipPosFound.end()) {
        // Create hit position queue.
        queue<Coordinate> hitPos;
        hitPos.push(Coordinate(x, y));
        shipPosFound[shipKey] = hitPos;

        // Create possible moves queue.
        queue<Coordinate> possibleMoves;
        
        // Up, down, left, right (respectively).
        // Adds positions around where the ship was hit.
        // We don't know where the ship is positioned at this point.
        if (y > 0)
            possibleMoves.push(Coordinate(x, y - 1));
        if (y < 9)
            possibleMoves.push(Coordinate(x, y + 1));
        if (x > 0)
            possibleMoves.push(Coordinate(x - 1, y));
        if (x < 9)
            possibleMoves.push(Coordinate(x + 1, y));
        cpuMoves[shipKey] = possibleMoves;

    // Otherwise, push the coordinates into the existing queues.
    } else {
        //Adjust cpuMoves.
        queue<Coordinate> &currMoves = cpuMoves[shipKey];

        // If the ship wasn't hit previously, then remove the blank moves (for this ship).
        if (shipPosFound[shipKey].size() == 1)
            currMoves = {};

        Coordinate prevShipMove = shipPosFound[shipKey].back();
        prevShipHit = thatShip; // Used if backtracking is needed.

        shipPosFound[shipKey].push(Coordinate(x, y));

        // If the coordinates found are going Up, down, left, right (respectively)
        // in respect with the previously hit position.
        if (y < prevShipMove.y) {
            // Continue the direction if it's still in bounds.
            if (y > 0) {
                currMoves.push(Coordinate(x, y - 1));
            } else {
                // Otherwise, set moves to sink the ship.
                setAltMoves(UP, x, y, prevShipMove);
            }
        } else if (y > prevShipMove.y) {
            if (y < 9) {
                currMoves.push(Coordinate(x, y + 1));
            } else {
                setAltMoves(DOWN, x, y, prevShipMove);
            }
        } else if (x < prevShipMove.x) {
            if (x > 0) {
                currMoves.push(Coordinate(x - 1, y));
            } else {
                setAltMoves(LEFT, x, y, prevShipMove);
            }
        } else if (x > prevShipMove.x) {
            if (x < 9) {
                currMoves.push(Coordinate(x + 1, y));
            } else {
                setAltMoves(RIGHT, x, y, prevShipMove);
            }
        }
    }
}

// Adjusts the possible moves when missing a shot while trying to sink a ship.
void Battleship::backTrackShot(int x, int y) {
    string shipKey = prevShipHit.name;
    Coordinate prevShipMove = shipPosFound[shipKey].back();

    // Used to determine new moves.
    int timesHit = shipPosFound[shipKey].size();

    // Check direction, then push in remaining moves to sink the ship.
    if (y < prevShipMove.y) {
        // Push moves to sink the ship.
        setAltMoves(UP, x, y, prevShipMove);
    } else if (y > prevShipMove.y) {
        setAltMoves(DOWN, x, y, prevShipMove);
    } else if (x < prevShipMove.x) {
        setAltMoves(LEFT, x, y, prevShipMove);
    } else if (x > prevShipMove.x) {
        setAltMoves(RIGHT, x, y, prevShipMove);
    }
}

// Sets the moves to sink a discovered ship.
void Battleship::setAltMoves(direction dir, int x, int y, Coordinate prevShipMove) {
    string shipKey = prevShipHit.name;
    queue<Coordinate> &currMoves = cpuMoves[shipKey];

    // Used to determine new moves.
    int timesHit = shipPosFound[shipKey].size();

    // Only consider ships with length 3 or more (A patrol boat would've been sunk anyway).
    switch (dir) {
        // If you went Up, go back Down.
        case UP:
            // Based on the ship's remaining health (it has been hit at least TWICE).
            switch (prevShipHit.health) {
                case 3:
                    currMoves.push(Coordinate(x, prevShipMove.y + (timesHit + 2)));
                case 2:
                    currMoves.push(Coordinate(x, prevShipMove.y + (timesHit + 1)));
                case 1:
                    currMoves.push(Coordinate(x, prevShipMove.y + timesHit));
            }
            break;
        // If you went Down, go back Up.
        case DOWN:
            switch (prevShipHit.health) {
                case 3:
                    currMoves.push(Coordinate(x, prevShipMove.y - (timesHit + 2)));
                case 2:
                    currMoves.push(Coordinate(x, prevShipMove.y - (timesHit + 1)));
                case 1:
                    currMoves.push(Coordinate(x, prevShipMove.y - timesHit));
            }
            break;
        // If you went Left, go Right.  
        case LEFT:
            switch (prevShipHit.health) {
                case 3:
                    currMoves.push(Coordinate(prevShipMove.x + (timesHit + 2), y));
                case 2:
                    currMoves.push(Coordinate(prevShipMove.x + (timesHit + 1), y));
                case 1:
                    currMoves.push(Coordinate(prevShipMove.x + timesHit, y));
            }
            break;
        // If you went Right, go Left.
        case RIGHT:
            switch (prevShipHit.health) {
                case 3:
                    currMoves.push(Coordinate(prevShipMove.x - (timesHit + 2), y));
                case 2:
                    currMoves.push(Coordinate(prevShipMove.x - (timesHit + 1), y));
                case 1:
                    currMoves.push(Coordinate(prevShipMove.x - timesHit, y));
            }
            break;
    }
}

// Sets the new previous ship, once a ship has sunk.
void Battleship::setPrevShip() {
    for (auto ship : shipPosFound) {
        queue<Coordinate> currQueue = ship.second;
        Ship currShip = p1Ships[ship.first[0]]; // First char of the ship name as a key.

        // Find any unsunk ships that were previously hit.
        if (currQueue.size() < currShip.length) {
            prevShipHit = currShip;
            return;
        }
    }
    prevShipHit.name = "";
}

// Checks if the game is over or not.
bool Battleship::isGameFinished() {
    return isFinished;
}

// Show the current contents of the boards.
void Battleship::showBoard() {
    cout << " P1  A B C D E F G H I J  |" << " CPU  A B C D E F G H I J" << endl;
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
                        cout << " | " << i + 1 << " | " << emptySpace << ' ';
                    } else if (j == 0) {
                        cout << " |  " << i + 1 << " | " << emptySpace << ' ';
                    } else {
                        cout << emptySpace << ' ';
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