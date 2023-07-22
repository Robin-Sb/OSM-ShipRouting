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
        void findTransitNodes();
        std::vector<Vec2Sphere> transformBack(); 
        std::vector<int> transitNodes;
    private:
        void fillBucketsVertical(Vec2 start, Vec2 end, int edgeIndex);
        void fillBucketsHorizontal(Vec2 start, Vec2 end, int edgeIndex);
        void findBoundaryNodes(int xIndex, int yIndex, int localIndex, std::array<std::vector<int>, 5> &cArray, std::array<std::vector<int>, 5> &indicesOfCArray, std::vector<std::pair<bool, std::tuple<int,int, RelativePosition>>> &boundaryNodes, int &n_boundaryNodes, RelativePosition relPos);
        void findBoundaryNodesDirectional(int xIndex, int yIndex, int localIndex, std::array<std::vector<int>, 5> &cArray, std::array<std::vector<int>, 5> &indicesOfCArray, std::vector<std::pair<bool, std::tuple<int,int, RelativePosition>>> &boundaryNodes, int &n_boundaryNodes, std::vector<std::vector<std::vector<int>>> &edgeBuckets, RelativePosition relPos);
        void storeDistances(int cellIndex, int vIndex, std::array<std::vector<int>, 5> &cArray, std::array<std::vector<int>, 5> &indicesOfCArray, std::vector<std::vector<std::pair<int, std::unordered_map<int, int>>>> &distancesVertical, std::array<std::unordered_map<int, int>, 2> &nodeToIdxMap, int lrIndex);
        bool compareYCoordinate(std::pair<int, std::unordered_map<int, int>> &value1, std::pair<int, std::unordered_map<int, int>> &value2);
        void dijkstra(int startIndex, std::vector<std::pair<bool, std::tuple<int,int, RelativePosition>>> &boundaryNodes, std::array<std::vector<int>, 5> &cLeft, std::array<std::vector<int>, 5> &cRight, int n_boundaryNodes);
        int gridsize;
        std::shared_ptr<Graph> graph;
        std::vector<std::vector<std::vector<int>>> edgeBucketsVertical;
        std::vector<std::vector<std::vector<int>>> edgeBucketsHorizontal;
};