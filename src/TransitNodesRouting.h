#pragma once
#include <vector>
#include "Graph.h"
#include <set>
#include <unordered_map>

class TransitNodesRouting {
    public:
        TransitNodesRouting(std::shared_ptr<Graph> _graph, int _gridsize);
        void findEdgeBuckets();
        void debug();
        void findTransitNodes();
        std::vector<Vec2Sphere> transformBack(); 
    private:
        void fillBucketsVertical(Vec2 start, Vec2 end, int edgeIndex);
        void fillBucketsHorizontal(Vec2 start, Vec2 end, int edgeIndex);
        void dijkstra(int startIndex, std::vector<std::pair<bool, std::pair<int,int>>> &boundaryNodes, std::array<std::vector<int>, 5> &cLeft, int n_boundaryNodes);
        int gridsize;
        std::shared_ptr<Graph> graph;
        std::vector<std::vector<std::vector<int>>> edgeBucketsVertical;
        std::vector<std::vector<std::vector<int>>> edgeBucketsHorizontal;
};