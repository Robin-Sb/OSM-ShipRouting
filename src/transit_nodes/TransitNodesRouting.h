#pragma once
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include "TransitNodesDef.h"
#include "../graph/Graph.h"
#include "SingleTnPass.h"

class TransitNodesRouting {
    public:
        TransitNodesRouting(std::shared_ptr<Graph> _graph, int _gridsize);
        void findEdgeBuckets();
        void debug();
        TransitNodesData sweepLineTransitNodesMain();
        std::vector<Vec2Sphere> getTransitNodesOfCell(int cellX, int cellY);
        std::string getTnList();
        std::vector<Vec2Sphere> transformBack(); 
        std::vector<int> transitNodes;
        std::vector<std::vector<int>> transitNodesDistances;
        // pair of transit node id and distance to it
        std::vector<std::vector<NodeDistance>> localTransitNodes;
    private:
        void fillBucketsVertical(Vec2 start, Vec2 end, int edgeIndex);
        void fillBucketsHorizontal(Vec2 start, Vec2 end, int edgeIndex);
        //void findBoundaryNodesHorizontal(int xIndex, int yIndex, std::vector<int> &cArray, std::vector<int> &indicesOfCArray, std::vector<std::pair<bool, std::pair<int, RelativePosition>>> &boundaryNodes, int &n_boundaryNodes, RelativePosition relPos);
        void findBoundaryNodesDirectional(int xIndex, int yIndex, std::vector<NodeDistance> &cArray, std::vector<BoundaryNodeData> &boundaryNodes, int &n_boundaryNodes, std::vector<std::vector<std::vector<int>>> &edgeBuckets, RelativePosition relPos, bool isVertical, std::vector<DistanceData> &nodeDistances, std::unordered_map<int, int>& nodeIdxToMapIdx);
        std::vector<NodeDistance> processSingleNodeVertical(int sweepIndexX, int sweepIndexY, int vIndex, std::vector<DistanceData> &distancesLeft, std::vector<DistanceData> &distancesRight, std::array<std::unordered_map<int, int>, 2>& nodeIdxToMapIdx);
        std::vector<NodeDistance> processSingleNodeHorizontal(int sweepIndexX, int sweepIndexY, int vIndex, std::vector<DistanceData> &distancesDown, std::vector<DistanceData> &distancesUp, std::array<std::unordered_map<int, int>, 2>& nodeIdxToMapIdx);
        
        void findTransitNodes(std::vector<DistanceData> &nodesLeft, std::vector<DistanceData> &nodesRight, std::unordered_set<int> &vs, std::unordered_map<int, std::vector<NodeDistance>> &distancesToNearestTransitNode, int sweepIndex, bool vertical);

        // sweepIndexX, vIndex, cRight, distancesRight, nodeIdxToMapIdx[1], 1
        void storeDistances(int cellIndexX, int cellIndexY, int vIndex, std::vector<NodeDistance> &cArray, std::vector<DistanceData> &distances, std::unordered_map<int, int> &nodeToIdxMap, bool vertical);
        std::vector<NodeDistance> dijkstra(int src, std::vector<BoundaryNodeData> &boundaryNodes, std::vector<NodeDistance> &cLeft, std::vector<NodeDistance> &cRight, int n_boundaryNodes);
        void computeDistancesBetweenTransitNodes(); 
        TransitNodesData postprocessTransitNodes();
        std::vector<int> dijkstraSSSP(int source);

        void collectTransitNodes();
        int gridsize;
        std::shared_ptr<Graph> graph;
        std::vector<std::vector<std::vector<int>>> edgeBucketsVertical;
        std::vector<std::vector<std::vector<int>>> edgeBucketsHorizontal;

        std::vector<std::vector<std::unordered_set<int>>> transitNodesOfCells;
        std::unordered_map<int, int> transitNodeTmp;
};