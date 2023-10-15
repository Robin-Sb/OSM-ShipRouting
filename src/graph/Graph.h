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
    ResultDTO(std::vector<int> &_path, int _distance): path(_path), distance(_distance) {};
    std::vector<int> path;
    int distance;
};


class Graph {
    public:
        std::vector<int> sources;
        std::vector<int> targets;
        std::vector<int> costs;
        std::vector<int> offsets;
        std::vector<Vec2Sphere> nodes;
        ResultDTO performDijkstraLogging(Vec2Sphere startPos, Vec2Sphere endPos);
        int getIndex(Vec2Sphere pos);
        void trim(int minLat, int maxLat, int minLon, int maxLon);
        void generate(int n, std::vector<SingleCoast> &coastlines);
        void generate(int n, std::vector<SingleCoast> &coastlines, float minLat, float maxLat, float minLon, float maxLon);
        void buildFromFMI(const std::string fmiFile);
        void readNodes(std::ifstream &file, int n);
        void readEdges(std::ifstream &file, int m);
        ResultDTO dijkstra(int startIndex, int endIndex);
    private:
        std::shared_ptr<SphericalGrid> sGrid;
};