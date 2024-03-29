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

        // index of the node which is checked
        int vIndex;

        // all the nodes along one sweepline (used to find them easily later in findTransitNodes)
        std::unordered_set<int> vs;
        // map which stores the results of multiple dijkstra passed
        // i.e. maps the index of all nodes in the cells c1...c5 to the potential transit nodes (aka all nodes in vs array)
        std::unordered_map<int, std::vector<NodeDistance>> distancesToNearestTransitNode;

        std::shared_ptr<Graph> graph;
        std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBucketsMain;
        std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBucketsSecondary;
        // current position of the sweepline
        int sweepIndexX;
        int sweepIndexY;
        int gridsize;
        // whether vertical or horizontal pass is executed, attempt to make everything more DRY
        bool vertical;
        // contains all the nodes in either the cell to the left (negative x) or down (negative y)
        std::vector<NodeDistance> cNegative;
        // same as before, but to the right or up (positive x and y)
        std::vector<NodeDistance> cPositive;
        // contains one entry for every node; indicates whether that node is a cell boundary and must be settled
        std::vector<BoundaryNodeData> boundaryNodes;

        // for a particular node, stores all the edges which are crossing a cell boundary
        // is used s.t. the transit node can be added to all cells the edge is part of  
        std::unordered_map<int, std::vector<int>> boundaryEdges;

        // stores for every node on the boundary of cells along one sweepline pass the distance to all vs
        // one could refactor this s.t. nodes of cNegative and cPositive are remapped to nodes in distanceToNearestTransitNodes
        // then those two vectors could be removed
        std::vector<DistanceData> distancesNegative;
        std::vector<DistanceData> distancesPositive;

        // maps the nodeindices (in graph.nodes) to the indices in distancesNegative and distancesPositive
        std::unordered_map<int, int> nodeIdxToMapIdxNegative;
        std::unordered_map<int, int> nodeIdxToMapIdxPositive;
};