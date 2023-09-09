#include <vector>
#include <random>
#include <thread>
#include <algorithm>
#include <iostream>
#include "../utils/Utils.h"
#include "../utils/GraphUtils.h"
#include "InPolyTest.h"


struct Edge {
    Edge (int _source, int _target, int _cost) {
        source = _source;
        target = _target;
        cost = _cost;
    }
    Edge() {}
    int source;
    int target;
    int cost;
};


class GraphGenerator {
    public:
        std::vector<int> sources;
        std::vector<int> targets;
        std::vector<int> costs;
        std::vector<Vec2Sphere> nodes;
        static const int range = 50000;
 
        void generate(int n, std::vector<SingleCoast> &coastlines, float minLat, float maxLat, float minLon, float maxLon);
        // void generate(int n, std::vector<SingleCoast> &coastlines);
        void performPolyTestsConcurrent(std::vector<Vec2Sphere> &allNodes, InPolyTest &polyTest, int n_threads);
        static void addNodeConcurrent(std::vector<Vec2Sphere> &allNodes, int rangeStart, int rangeEnd, InPolyTest &polyTest, std::shared_ptr<std::vector<Vec2Sphere>> outNodes);
        static void findEdgeConcurrent(std::vector<Vec2Sphere> &allNodes, int startIndex, int endIndex, SphericalGrid &grid, std::shared_ptr<std::vector<Edge>>); // std::shared_ptr<std::vector<int>> _sources, std::shared_ptr<std::vector<int>> _targets, std::shared_ptr<std::vector<int>> _costs
        void performEdgeSearchConcurrent(int n_threads);

    private:
        std::unique_ptr<SphericalGrid> sGrid;
};