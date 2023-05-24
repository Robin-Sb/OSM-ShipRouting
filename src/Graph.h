#pragma once
#include <vector>
#include <random>
#include "Utils.h"
#include "GraphUtils.h"
#include "InPolyTest.h"

class Graph {
    public:
        Graph(std::vector<SingleCoast> _coastlines);
        InPolyTest polyTest;
        std::vector<int> sources;
        std::vector<int> targets;
        std::vector<int> costs;
        std::vector<int> offsets;
        std::vector<Vec2Sphere> nodes;
        std::vector<Vec2Sphere> drawNodes;
        void generate(int n);
    private:
};