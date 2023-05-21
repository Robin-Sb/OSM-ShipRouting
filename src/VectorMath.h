#pragma once
#include <vector>

class Vec2Sphere {
    public:
        Vec2Sphere() {};
        Vec2Sphere(float lat, float lon);
        float lat;
        float lon;
};

class SphericalGrid {
    public:
        void addPoint(int nodeIndex);
        std::vector<int> cells[360][180];
};