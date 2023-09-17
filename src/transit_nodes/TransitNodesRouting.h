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
        TransitNodesData sweepLineTransitNodesMain();
        std::vector<Vec2Sphere> transformBack();
        // stores transit nodes
        std::vector<int> transitNodes;
        // stores distances between transit nodes
        std::vector<std::vector<int>> transitNodesDistances;
        // pair of transit node id and distance to it
        std::vector<std::vector<NodeDistance>> localTransitNodes;
    private:
        void fillBucketsVertical(Vec2 start, Vec2 end, int edgeIndex);
        void fillBucketsHorizontal(Vec2 start, Vec2 end, int edgeIndex);
        
        void computeDistancesBetweenTransitNodes(); 
        TransitNodesData postprocessTransitNodes();
        std::vector<int> dijkstraSSSP(int source);

        void collectTransitNodes();
        int gridsize;
        std::shared_ptr<Graph> graph;
        // stores all edges which cross the boundary of a gridcell in vertical direction
        std::vector<std::vector<std::vector<int>>> edgeBucketsVertical;
        // same in horizontal direction
        std::vector<std::vector<std::vector<int>>> edgeBucketsHorizontal;

        // for every cell, stores its transit nodes
        std::vector<std::vector<std::unordered_set<int>>> transitNodesOfCells;
        // map from each transit node to its global node index
        std::unordered_map<int, int> transitNodeTmp;
};