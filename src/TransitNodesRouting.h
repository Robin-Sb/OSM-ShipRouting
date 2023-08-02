#pragma once
#include <vector>
#include "Graph.h"
#include <set>
#include <unordered_map>

enum RelativePosition {
    LEFTLOWER,
    RIGHTUPPER,
    UNDEF
};

class TransitNodesRouting {
    public:
        TransitNodesRouting(std::shared_ptr<Graph> _graph, int _gridsize);
        void findEdgeBuckets();
        void debug();
        void sweepLineTransitNodes();
        std::vector<Vec2Sphere> transformBack(); 
        std::vector<int> transitNodes;
        std::vector<std::vector<int>> transitNodesDistances;
        // pair of transit node id and distance to it
        std::vector<std::vector<std::pair<int, int>>> localTransitNodes;
    private:
        void fillBucketsVertical(Vec2 start, Vec2 end, int edgeIndex);
        void fillBucketsHorizontal(Vec2 start, Vec2 end, int edgeIndex);
        void findBoundaryNodes(int xIndex, int yIndex, int localIndex, std::array<std::vector<int>, 5> &cArray, std::array<std::vector<int>, 5> &indicesOfCArray, std::vector<std::pair<bool, std::tuple<int,int, RelativePosition>>> &boundaryNodes, int &n_boundaryNodes, RelativePosition relPos);
        void findBoundaryNodesHorizontal(int xIndex, int yIndex, std::vector<int> &cArray, std::vector<int> &indicesOfCArray, std::vector<std::pair<bool, std::pair<int, RelativePosition>>> &boundaryNodes, int &n_boundaryNodes, RelativePosition relPos);
        void findBoundaryNodesDirectional(int xIndex, int yIndex, std::vector<int> &cArray, std::vector<int> &indicesOfCArray, std::vector<std::pair<bool, std::pair<int, RelativePosition>>> &boundaryNodes, int &n_boundaryNodes, std::vector<std::vector<std::vector<int>>> &edgeBuckets, RelativePosition relPos);
        void processSingleNode(int sweepIndexX, int sweepIndexY, int edgeIndex, std::vector<std::vector<std::pair<int, std::unordered_map<int, int>>>>& distancesVertical, std::array<std::unordered_map<int, int>, 2>& nodeIdxToMapIdx);
        void findTransitNodes(std::vector<std::vector<std::pair<int, std::unordered_map<int, int>>>> &distances);

        void storeDistances(int cellIndex, int vIndex, std::vector<int> &cArray, std::vector<int> &indicesOfCArray, std::vector<std::vector<std::pair<int, std::unordered_map<int, int>>>> &distancesVertical, std::array<std::unordered_map<int, int>, 2> &nodeToIdxMap, int lrIndex);
        bool compareCoordinates(std::pair<int, std::unordered_map<int, int>> &value1, std::pair<int, std::unordered_map<int, int>> &value2, bool sortByY);
        void dijkstra(int startIndex, std::vector<std::pair<bool, std::pair<int, RelativePosition>>> &boundaryNodes, std::vector<int> &cLeft, std::vector<int> &cRight, int n_boundaryNodes);
        void computeDistancesBetweenTransitNodes(); 
        void sortDescending(std::vector<std::vector<std::pair<int, std::unordered_map<int, int>>>> &distances, bool sortByY);
        std::vector<int> dijkstraSSSP(int source);

        int gridsize;
        std::shared_ptr<Graph> graph;
        std::vector<std::vector<std::vector<int>>> edgeBucketsVertical;
        std::vector<std::vector<std::vector<int>>> edgeBucketsHorizontal;
};