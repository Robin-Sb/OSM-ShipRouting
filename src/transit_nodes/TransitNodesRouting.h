#pragma once
#include <vector>
#include "../graph/Graph.h"
#include <set>
#include <unordered_map>
#include <unordered_set>

enum RelativePosition {
    LEFTLOWER,
    RIGHTUPPER,
    UNDEF
};

struct NodeDistance {
    NodeDistance(int _nodeIndex, int _distanceToV) {
        nodeIndex = _nodeIndex;
        distanceToV = _distanceToV;
    }
    int nodeIndex;
    int distanceToV;
};

struct DistanceData {
    DistanceData(int _nodeId, std::unordered_map<int, int> _distanceToV) {
        referenceNodeIndex = _nodeId;
        distanceToV = _distanceToV;
    }
    int referenceNodeIndex;
    std::unordered_map<int, int> distanceToV;
};

struct BoundaryNodeData {
    BoundaryNodeData() {
        isAtCellBoundary = false;
        localIndex = -1;
        relPos = RelativePosition::UNDEF;
    }
    BoundaryNodeData(bool _isAtCellBoundary, int _localIndex, RelativePosition _relPos) {
        isAtCellBoundary = _isAtCellBoundary;
        localIndex = _localIndex;
        relPos = _relPos;
    }
    bool isAtCellBoundary;
    int localIndex;
    RelativePosition relPos;
};

struct TransitNodesData {
    TransitNodesData(std::vector<int> _transitNodes, std::vector<std::vector<int>> _distancesBetweenTransitNodes, std::vector<std::vector<std::vector<int>>> _transitNodesPerCell, std::vector<std::vector<int>> _distancesToLocalTransitNodes) {
        transitNodes = _transitNodes;
        distancesBetweenTransitNodes = _distancesBetweenTransitNodes;
        transitNodesPerCell = _transitNodesPerCell;
        distancesToLocalTransitNodes = _distancesToLocalTransitNodes;
    }
    std::vector<int> transitNodes;
    std::vector<std::vector<int>> distancesBetweenTransitNodes;
    std::vector<std::vector<std::vector<int>>> transitNodesPerCell;
    std::vector<std::vector<int>> distancesToLocalTransitNodes;
};

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
        void findBoundaryNodesHorizontal(int xIndex, int yIndex, std::vector<int> &cArray, std::vector<int> &indicesOfCArray, std::vector<std::pair<bool, std::pair<int, RelativePosition>>> &boundaryNodes, int &n_boundaryNodes, RelativePosition relPos);
        void findBoundaryNodesDirectional(int xIndex, int yIndex, std::vector<NodeDistance> &cArray, std::vector<BoundaryNodeData> &boundaryNodes, int &n_boundaryNodes, std::vector<std::vector<std::vector<int>>> &edgeBuckets, RelativePosition relPos);
        std::vector<NodeDistance> processSingleNodeVertical(int sweepIndexX, int sweepIndexY, int vIndex, std::vector<DistanceData> &distancesLeft, std::vector<DistanceData> &distancesRight, std::array<std::unordered_map<int, int>, 2>& nodeIdxToMapIdx);
        std::vector<NodeDistance> processSingleNodeHorizontal(int sweepIndexX, int sweepIndexY, int vIndex, std::vector<DistanceData> &distancesDown, std::vector<DistanceData> &distancesUp, std::array<std::unordered_map<int, int>, 2>& nodeIdxToMapIdx);
        
        void findTransitNodes(std::vector<DistanceData> &nodesLeft, std::vector<DistanceData> &nodesRight, std::vector<int> &vs, std::unordered_map<int, std::vector<NodeDistance>> &distancesToNearestTransitNode, int sweepIndex, bool vertical);

        // sweepIndexX, vIndex, cRight, distancesRight, nodeIdxToMapIdx[1], 1
        void storeDistances(int cellIndex, int vIndex, std::vector<NodeDistance> &cArray, std::vector<DistanceData> &distances, std::unordered_map<int, int> &nodeToIdxMap);
        bool compareCoordinates(DistanceData &value1, DistanceData &value2, bool sortByY);
        std::vector<NodeDistance> dijkstra(int src, std::vector<BoundaryNodeData> &boundaryNodes, std::vector<NodeDistance> &cLeft, std::vector<NodeDistance> &cRight, int n_boundaryNodes);
        void computeDistancesBetweenTransitNodes(); 
        void sortDescending(std::vector<DistanceData> &distances, bool sortByY);
        std::vector<int> dijkstraSSSP(int source);

        TransitNodesData postprocessTransitNodes();

        int gridsize;
        std::shared_ptr<Graph> graph;
        std::vector<std::vector<std::vector<int>>> edgeBucketsVertical;
        std::vector<std::vector<std::vector<int>>> edgeBucketsHorizontal;

        std::vector<std::vector<std::unordered_set<int>>> transitNodesOfCells;
        std::unordered_map<int, int> transitNodeTmp;

        static int getCellX(Vec2Sphere &v, int gridX);
        static int getCellY(Vec2Sphere &v, int gridY);

        bool sameCell(Vec2Sphere &v1, Vec2Sphere &v2);

};