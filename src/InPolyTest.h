#pragma once
#include <vector>
#include "Graph.h"
#include "VectorMath.h"
#include "math.h"

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


class InPolyTest {
    public: 
        static Location isPointInPolygon(std::vector<Node> polygon, Vec2Sphere point);
        static float transformLongitude(Vec2Sphere p, Vec2Sphere q);
        static CardinalDirection isEastOrWest(float lonA, float lonB);
};