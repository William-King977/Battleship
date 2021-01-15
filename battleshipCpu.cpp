#include "battleshipCpu.hpp"
#include <iostream>
using namespace std;

BattleshipCPU::BattleshipCPU() {
    // cout << "BattleshipCPU object made." << endl;
    probBoard = new int* [10];
    for (int i = 0; i < 10; i++) {
        probBoard[i] = new int[10];
        for (int j = 0; j < 10; j++) {
            probBoard[i][j] = 0;
        }
    }
    sinkMode = false;
}

// Deconstructor deletes/clears certain data structures.
BattleshipCPU::~BattleshipCPU() {
    // cout << "BattleshipCPU object destroyed." << endl;
    for (int i = 0; i < 10; i++) {
        delete[] probBoard[i];
    }

    delete[] probBoard;
    cpuMoves = {};
    shipPosFound = {};
}

// Performs the CPU's turn.
void BattleshipCPU::cpuShoot() {
    int x;
    int y;
    
    // Get a move from CPU moves if possible.
    if (!cpuMoves.empty()) {
        Coordinate nextMove = getCpuMove();
        x = nextMove.getX();
        y = nextMove.getY();
    } else {
        // Use probabilty density function to find the next move.
        Coordinate nextMove = getNextMove();
        x = nextMove.getX();
        y = nextMove.getY();
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
            cout << "Miss." << endl;
            // If there is a ship that has been hit (but not sunk),
            // then push the remaining moves to sink it.
            if (sinkMode) {
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

        // If the resulting hit sunk the ship.
        if (thatShip.getHealth() == 0) {
            cout << "Hit and sunk. " << thatShip.getName() << '.' << endl;
            p1ShipCount--;
            // Remove ship from the unordered maps.
            shipPosFound.erase(thatShip.getName());
            cpuMoves.erase(thatShip.getName());
            // Sets the next previous ship to sink.
            setPrevShip();
            sinkMode = false;
        // Only add moves if the ship has not sunk.
        } else {
            cout << "Hit. " << thatShip.getName() << '.' << endl;
            setCpuMoves(x, y, thatShip);
        }
    }

    // Show the number of ships sunk.
    cout << "Ships Sunk: " << (5 - p1ShipCount) << endl;

    // If all the ships have sunk.
    if (p1ShipCount == 0) {
        p2Win = true;
    }
}

// Calculate the probability of each position holding an unsunk ship.
void BattleshipCPU::calculateProbability() {
    // Go through the board.
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            // Reset the position.
            probBoard[i][j] = 0;

            // Skip positions already hit.
            if (isPosHit(p1Board[i][j])) {
                continue;
            }

            // Go through each unsunk ship.
            for (auto elem : p1Ships) {
                if (elem.second.getHealth() == 0) {
                    continue;
                }

                int shipLength = elem.second.getLength();
                bool upInBound = true;
                bool downInBound = true;
                bool leftInBound = true;
                bool rightInBound = true;

                // Check if the ship is placeable in each direction.
                for (int k = 1; k < shipLength; k++) {
                    int upPos = i - k;
                    int downPos = i + k;
                    int leftPos = j - k;
                    int rightPos = j + k;

                    // UP.
                    if (upPos < 0 || isPosHit(p1Board[upPos][j])) {
                        upInBound = false;
                    }
                    // DOWN
                    if (downPos > 9 || isPosHit(p1Board[downPos][j])) {
                        downInBound = false;
                    }
                    // LEFT
                    if (leftPos < 0 || isPosHit(p1Board[i][leftPos])) {
                        leftInBound = false;
                    }
                    // RIGHT
                    if (rightPos > 9 || isPosHit(p1Board[i][rightPos])) {
                        rightInBound = false;
                    }
                    // Exit early if possible.
                    if (!upInBound && !downInBound && !leftInBound && !rightInBound) {
                        break;
                    }
                }

                // Increment where appropriate.
                if (upInBound) {
                    probBoard[i][j]++;
                }
                if (downInBound) {
                    probBoard[i][j]++;
                }
                if (leftInBound) {
                    probBoard[i][j]++;
                }
                if (rightInBound) {
                    probBoard[i][j]++;
                }
            }
        }
    }
}

// Checks for even parity for a specified position.
bool BattleshipCPU::checkParity(int x, int y) {
    int minShipSize = 5;
    // Get the size of the smallest unsunk ship.
    for (auto ship : p1Ships) {
        Ship currShip = ship.second;
        if (currShip.getHealth() > 0 && currShip.getLength() < minShipSize) {
            minShipSize = currShip.getLength();
        }
    }

    bool validParity = false;
    int startPos = (minShipSize - 1) - y;

    // Adjust the starting position until it's positive.
    while (startPos < 0) {
        startPos+= minShipSize;
    }

    // Check if it's a parity based on the x coordinate.
    for (int j = startPos; j < 10; j+=minShipSize) {
        if (j == x) {
            validParity = true;
            break;
        }
    }
    return validParity;
}

// Gets the move with the highest density probability.
Coordinate BattleshipCPU::getNextMove() {
    // Find largest probability and use that as the next move.
    calculateProbability();
    Coordinate nextMove(-1, -1);
    int currMax = 0;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            // Favour positions with even parity.
            if ((probBoard[i][j] >= currMax) && checkParity(j, i)) {
                currMax = probBoard[i][j];
                nextMove = Coordinate(j, i);
            } else if (probBoard[i][j] > currMax) {
                currMax = probBoard[i][j];
                nextMove = Coordinate(j, i);
            }
        }
    }
    return nextMove;
}

// Gets a move used to hunt down a discovered ship.
Coordinate BattleshipCPU::getCpuMove() {
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
        sinkMode = true;
        return getCpuMove();
    }

    // If the CPU is trying to find the ship's position.
    if (!sinkMode) {
        Coordinate prevMove = shipPosFound[shipKey].front();
        Coordinate newMove = shipMoves.front();
        Direction dir = getDirection(prevMove, newMove);
        // If it's not possible for the ship to be in the given direction.
        if (!canShipExist(p1Ships[shipKey[0]].getLength(), prevMove, dir)) {
            shipMoves.pop();
            return getCpuMove();
        }
    }

    Coordinate nextMove = shipMoves.front();
    shipMoves.pop();
    return nextMove;
}

// Sets the possible moves for the CPU after a ship is hit.
void BattleshipCPU::setCpuMoves(int x, int y, Ship thatShip) {
    // If the key (ship) doesn't exist, try and find the ship's direction.
    if (shipPosFound.find(thatShip.getName()) == shipPosFound.end()) {
        findShip(x, y, thatShip);
        sinkMode = false;
    // Otherwise, push the coordinates into the existing queues (to sink the ship).
    } else {
        sinkShip(x, y, thatShip);
        sinkMode = true;
    }
}

// Pushes in moves that are used to find a ship's direction.
void BattleshipCPU::findShip(int x, int y, Ship thatShip) {
    // Create hit position queue (positions hit on the ship).
    queue<Coordinate> shipHitPos;
    shipHitPos.push(Coordinate(x, y));
    shipPosFound[thatShip.getName()] = shipHitPos;

    // Create possible moves queue (for cpuMoves).
    queue<Coordinate> possibleMoves;

    bool upPlaced = false;
    bool downPlaced = false;
    bool leftPlaced = false;
    bool rightPlaced = false;

    // Set probability to -1 if the position is out of bounds.
    int upProb = (y > 0) && !isPosHit(p1Board[y - 1][x]) ? probBoard[y - 1][x] : -1;
    int downProb = (y < 9) && !isPosHit(p1Board[y + 1][x]) ? probBoard[y + 1][x] : -1;
    int leftProb = (x > 0) && !isPosHit(p1Board[y][x - 1]) ? probBoard[y][x - 1] : -1;
    int rightProb = (x < 9) && !isPosHit(p1Board[y][x + 1]) ? probBoard[y][x + 1] : -1;

    // Adds positions around where the ship was hit (in descending order of probability).
    // We don't know where the ship is positioned at this point.
    while (!upPlaced || !downPlaced || !leftPlaced || !rightPlaced) {
        int maxVerti = (upProb > downProb) ? upProb : downProb;
        int maxHoriz = (leftProb > rightProb) ? leftProb : rightProb;
        int currMax = (maxVerti > maxHoriz) ? maxVerti : maxHoriz;

        // UP.
        if (y > 0 && !isPosHit(p1Board[y - 1][x]) && !upPlaced) {
            if (upProb == currMax) {
                possibleMoves.push(Coordinate(x, y - 1));
                upPlaced = true;
                upProb = -1;
            }
        } else {
            // Even if it's not valid. Indicate that it has been checked.
            upPlaced = true;
        }

        // DOWN.
        if (y < 9 && !isPosHit(p1Board[y + 1][x]) && !downPlaced) {
            if (downProb == currMax) {
                possibleMoves.push(Coordinate(x, y + 1));
                downPlaced = true;
                downProb = -1;
            }
        } else {
            downPlaced = true;
        }

        // LEFT.
        if (x > 0 && !isPosHit(p1Board[y][x - 1]) && !leftPlaced) {
            if (leftProb == currMax) {
                possibleMoves.push(Coordinate(x - 1, y));
                leftPlaced = true;
                leftProb = -1;
            }
        } else {
            leftPlaced = true;
        }

        // RIGHT.
        if (x < 9 && !isPosHit(p1Board[y][x + 1]) && !rightPlaced) {
            if (rightProb == currMax) {
                possibleMoves.push(Coordinate(x + 1, y));
                rightPlaced = true;
                rightProb = -1;
            }
        } else {
            rightPlaced = true;
        }
    }
    // Set the cpuMoves queue.
    cpuMoves[thatShip.getName()] = possibleMoves;
}

// Push in moves that are used to sink a ship.
void BattleshipCPU::sinkShip(int x, int y, Ship thatShip) {
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
void BattleshipCPU::backTrackShot(int x, int y) {
    string shipKey = prevShipHit.getName();
    Coordinate prevShipMove = shipPosFound[shipKey].back();

    // Get direction, then push in remaining moves to sink the ship.
    Direction dir = getDirection(Coordinate(x, y), prevShipMove);
    setAltMoves(dir, prevShipMove);
}

// Sets the moves to sink a discovered ship.
void BattleshipCPU::setAltMoves(Direction dir, Coordinate prevShipMove) {
    string shipKey = prevShipHit.getName();
    queue<Coordinate> &currMoves = cpuMoves[shipKey];

    int x = prevShipMove.getX();
    int y = prevShipMove.getY();

    // Used to determine new moves.
    int timesHit = shipPosFound[shipKey].size(); // Could use length - health...

    // Only consider ships with length 3 or more (A patrol boat would've been sunk already).
    switch (dir) {
        // If you went Up, go Down.
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
        // If you went Down, go Up.
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
void BattleshipCPU::setPrevShip() {
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
    }
}

// Returns a direction based on two coordinates of a ship.
Direction BattleshipCPU::getDirection(Coordinate first, Coordinate last) {
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
bool BattleshipCPU::canShipExist(int shipLength, Coordinate currPos, Direction dir) {
    bool upperInBounds = true;
    bool lowerInBounds = true;
    // Initialise them with Y if it's vertical, otherwise X.
    int upperBound = (dir == UP || dir == DOWN) ? currPos.getY() : currPos.getX();
    int lowerBound = (dir == UP || dir == DOWN) ? currPos.getY() : currPos.getX();
    
    switch (dir) {
        case UP:
        case DOWN:
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
    int innerBound = (upperBound - lowerBound) + 1;
    bool placementPossible = (innerBound) >= shipLength ? true : false;
    return placementPossible;
}