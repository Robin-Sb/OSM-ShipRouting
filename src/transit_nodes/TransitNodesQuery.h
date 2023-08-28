#pragma once
#include "../graph/Graph.h"
#include "TransitNodesDef.h"
#include "../utils/Utils.h"

class TransitNodesQuery {
    public:
        TransitNodesQuery(std::shared_ptr<Graph> _graph, TransitNodesData _tnData);
        int query(int sources, int target);
    private:
        std::shared_ptr<Graph> graph;
        TransitNodesData tnData;
};