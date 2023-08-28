#pragma once
#include <vector>
#include <fstream>
#include <queue>
#include <set>
#include <iostream>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include "../utils/Utils.h"
#include "../utils/GraphUtils.h"
#include "GraphGenerator.h"


struct ResultDTO {
    ResultDTO(std::vector<Vec2Sphere> &_path, int _distance): path(_path), distance(_distance) {};
    std::vector<Vec2Sphere> path;
    int distance;
};


class Graph {
    public:
        std::vector<int> sources;
        std::vector<int> targets;
        std::vector<int> costs;
        std::vector<int> offsets;
        std::vector<Vec2Sphere> nodes;
        ResultDTO performDijkstraMultiple(int start, std::set<int> endNodes);
        ResultDTO performDijkstraLogging(Vec2Sphere startPos, Vec2Sphere endPos);
        void trim(int minLat, int maxLat, int minLon, int maxLon);
        void generate(int n, std::vector<SingleCoast> &coastlines);
        void buildFromFMI(std::string fmiFile);
        void readNodes(std::ifstream &file, int n);
        void readEdges(std::ifstream &file, int m);
        ResultDTO dijkstra(int startIndex, int endIndex);
        // void performPolyTestsConcurrent(std::vector<Vec2Sphere> &allNodes, InPolyTest &polyTest, int n_threads);
        // static void addNodeConcurrent(std::vector<Vec2Sphere> &allNodes, int rangeStart, int rangeEnd, InPolyTest &polyTest, std::shared_ptr<std::vector<Vec2Sphere>> outNodes);
        // static void findEdgeConcurrent(std::vector<Vec2Sphere> &allNodes, int startIndex, int endIndex, SphericalGrid &grid,  std::shared_ptr<std::vector<int>> _sources, std::shared_ptr<std::vector<int>> _targets, std::shared_ptr<std::vector<int>> _costs);
        // void performEdgeSearchConcurrent(int n_threads);
    private:
        std::shared_ptr<SphericalGrid> sGrid;
};