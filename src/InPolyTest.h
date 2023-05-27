#pragma once
#include <vector>
#include "Utils.h"
#include "math.h"
#include "GraphUtils.h"

enum CardinalDirection {
    EAST = 1,
    WEST = 2,
    NONE = 3
};

enum Location {
    OUTSIDE = 0,
    INSIDE = 1,
    BOUNDARY = 2,
    ERROR = 3
};

class BoundingPolygon {
    public:
        BoundingPolygon(float _latMin, float _latMax, float _lonMin, float _lonMax);
        float latMin;
        float latMax;
        float lonMin;
        float lonMax;
        bool isInside(Vec2Sphere point);
};

class InPolyTest {
    public: 
        InPolyTest() {};
        InPolyTest(std::vector<SingleCoast> _coastlines);
        std::vector<SingleCoast> coastlines;
        bool performPointInPolyTest(Vec2Sphere point);

    private: 
        std::vector<BoundingPolygon> bps;
        Location isPointInPolygon(std::vector<Node> &polygon, Vec2Sphere point);
        float transformLongitude(Vec2Sphere p, Vec2Sphere q);
        CardinalDirection isEastOrWest(float lonA, float lonB);
};