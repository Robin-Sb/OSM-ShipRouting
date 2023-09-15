#pragma once
#include "../graph/Graph.h"
#include "TransitNodesDef.h"
#include "../utils/Utils.h"

class TransitNodesQuery {
    public:
        TransitNodesQuery(std::shared_ptr<Graph> _graph, std::shared_ptr<TransitNodesData> _tnData);
        int query(int source, int target);
        ResultDTO path_query(int source, int target);
        bool lessThanNGridCellsAway(int u, int v, int n);
    private:
        std::shared_ptr<Graph> graph;
        std::shared_ptr<TransitNodesData> tnData;
        int query_alg(int source, int target);
};