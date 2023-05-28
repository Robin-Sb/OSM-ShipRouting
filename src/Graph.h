#pragma once
#include <vector>
#include <random>
#include "Utils.h"
#include "GraphUtils.h"
#include "InPolyTest.h"
#include <fstream>
#include <thread>
#include <queue>
#include <set>
#include <algorithm>


class Graph {
    public:
        std::vector<int> sources;
        std::vector<int> targets;
        std::vector<int> costs;
        std::vector<int> offsets;
        std::vector<Vec2Sphere> nodes;
        std::vector<Vec2Sphere> performDijkstra(Vec2Sphere startPos, Vec2Sphere endPos);
        void generate(int n, std::vector<SingleCoast> coastlines);
        void buildFromFMI(std::string fmiFile);
        void readNodes(std::ifstream &file, int n);
        void readEdges(std::ifstream &file, int m);
        void performPolyTestsConcurrent(std::vector<Vec2Sphere> &allNodes, InPolyTest &polyTest, int n_threads);
        static void addNodeConcurrent(std::vector<Vec2Sphere> &allNodes, int rangeStart, int rangeEnd, InPolyTest &polyTest, std::shared_ptr<std::vector<Vec2Sphere>> outNodes);
        static void findEdgeConcurrent(std::vector<Vec2Sphere> &allNodes, int startIndex, int endIndex, SphericalGrid &grid,  std::shared_ptr<std::vector<int>> _sources, std::shared_ptr<std::vector<int>> _targets, std::shared_ptr<std::vector<int>> _costs);
        void performEdgeSearchConcurrent(int n_threads);
    private:
        std::unique_ptr<SphericalGrid> sGrid;
        std::vector<int> dijkstra(int startIndex, int endIndex);
};