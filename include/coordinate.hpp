#ifndef COORDINATE_HPP
#define COORDINATE_HPP

class Coordinate {
    private:
        int x;
        int y;

    public:
        Coordinate(int x, int y);
        ~Coordinate();

        int getX();
        int getY();
};

#endif