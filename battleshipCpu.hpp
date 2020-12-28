#ifndef BATTLESHIPCPU_HPP
#define BATTLESHIPCPU_HPP

#include "battleship.hpp"
#include <queue>

class BattleshipCPU : public Battleship {
    public:
        BattleshipCPU();
        ~BattleshipCPU();
        void cpuShoot();
    protected:
        int** probBoard; // For CPU probability.
        bool sinkMode; // True, if it's currently sinking a found ship.
        Ship prevShipHit;
        unordered_map<string, queue<Coordinate>> shipPosFound; // Discovered ship positions.
        unordered_map<string, queue<Coordinate>> cpuMoves; // Moves to sink the ship(s) found.

        // Methods.
        void calculateProbability();
        void resetProbability();
        Coordinate getNextMove(); // Get move based on probability density.
        Coordinate getCpuMove();
        void setCpuMoves(int x, int y, Ship thatShip);
        void findShip(int x, int y, Ship thatShip);
        void sinkShip(int x, int y, Ship thatShip);
        
        void backTrackShot(int x, int y);
        void setAltMoves(Direction dir, Coordinate prevShipMove);
        void setPrevShip();
        Direction getDirection(Coordinate first, Coordinate last);
        bool canShipExist(int shipLength, Coordinate currPos, Direction dir);
};

#endif