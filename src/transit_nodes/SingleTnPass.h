#pragma once
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <array>
#include "TransitNodesDef.h"
#include "../graph/Graph.h"

class SingleTnPass {
    public:
        SingleTnPass(int _sweepIndexX, int _sweepIndexY, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> _edgeBucketsMain, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> _edgeBucketsSecondary, std::shared_ptr<Graph> _graph, int _gridsize, bool vertical);
        void findTransitNodes(std::vector<std::vector<std::unordered_set<int>>> &transitNodesOfCells, std::unordered_map<int, int> &transitNodeTmp);
        void assignDistances(std::unordered_map<int, int> &transitNodeTmp, std::vector<std::vector<std::unordered_set<int>>> &transitNodesOfCells, std::vector<std::vector<NodeDistance>> &localTransitNodes);
        void singleSweepLinePass();
    private: 
        std::vector<NodeDistance> processSingleNode(int vIndex);
        std::vector<NodeDistance> dijkstra();
        void searchVertical(int i, int cell, bool verticalPass);
        void searchHorizontal(int i, int cell, bool horizontalPass);
        void findBoundaryNodesNegative(int xIndex, int yIndex, bool verticalPass, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets);
        void findBoundaryNodesPositive(int xIndex, int yIndex, bool verticalPass, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets);
        void storeDistancesNegative();
        void storeDistancesPositive();


        std::pair<int, int> orderNodes(int startIndex, int endIndex, bool verticalPass);
        // index of the node which is checked
        int vIndex;
        std::unordered_map<int, int> nodeIdxToMapIdxNegative;
        std::unordered_map<int, int> nodeIdxToMapIdxPositive;

        std::unordered_set<int> vs;
        std::unordered_map<int, std::vector<NodeDistance>> distancesToNearestTransitNode;

        std::shared_ptr<Graph> graph;
        std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBucketsMain;
        std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBucketsSecondary;
        // current position of the sweepline
        int sweepIndexX;
        int sweepIndexY;
        int gridsize;
        bool vertical;
        // contains all the nodes in either the cell to the left (negative x) or down (negative y)
        std::vector<NodeDistance> cNegative;
        // same as before, but to the right or up (positive x and y)
        std::vector<NodeDistance> cPositive;
        // contains one entry for every node; indicates whether that node is a cell boundary and must be settled
        std::vector<BoundaryNodeData> boundaryNodes;
        int n_boundaryNodes;

        std::vector<DistanceData> distancesNegative;
        std::vector<DistanceData> distancesPositive;

};