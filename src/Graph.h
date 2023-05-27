#pragma once
#include <vector>
#include <random>
#include "Utils.h"
#include "GraphUtils.h"
#include "InPolyTest.h"
#include <fstream>
#include <thread>
#include <future>

class Graph {
    public:
        std::vector<int> sources;
        std::vector<int> targets;
        std::vector<int> costs;
        std::vector<int> offsets;
        std::vector<Vec2Sphere> nodes;
        std::vector<Vec2Sphere> drawNodes;
        void generate(int n, std::vector<SingleCoast> coastlines);
        void buildFromFMI(std::string fmiFile);
        void readNodes(std::ifstream &file, int n);
        void readEdges(std::ifstream &file, int m);
        void performPolyTestsConcurrent(std::vector<Vec2Sphere> &allNodes, InPolyTest &polyTest);
        static void addNodeConcurrent(std::vector<Vec2Sphere> &allNodes, int rangeStart, int rangeEnd, InPolyTest &polyTest, std::shared_ptr<std::vector<Vec2Sphere>> outNodes);
    private:
};