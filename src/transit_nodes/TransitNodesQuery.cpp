#include "TransitNodesQuery.h"

TransitNodesQuery::TransitNodesQuery(std::shared_ptr<Graph> _graph, std::shared_ptr<TransitNodesData> _tnData) {
    graph = _graph;
    tnData = _tnData;
}

// query; loop over all pairs of transit nodes from source and target and take the minimum
int TransitNodesQuery::query_alg(int source, int target) {
    int srcCellX = UtilFunctions::getCellX(graph->nodes[source], tnData->gridsize_x);
    int trgCellX = UtilFunctions::getCellX(graph->nodes[target], tnData->gridsize_x);
    int srcCellY = UtilFunctions::getCellY(graph->nodes[source], tnData->gridsize_y);
    int trgCellY = UtilFunctions::getCellY(graph->nodes[target], tnData->gridsize_y);
    
    long minDist = 2000000000;
    
    for (int i = 0; i < tnData->distancesToLocalTransitNodes[source].size(); i++) {
        for (int j = 0; j < tnData->distancesToLocalTransitNodes[target].size(); j++) {
            int srcToTnDist = tnData->distancesToLocalTransitNodes[source][i];
            int trgToTnDist = tnData->distancesToLocalTransitNodes[target][j];
            int srcTn = tnData->transitNodesPerCell[srcCellX][srcCellY][i];
            int trgTn = tnData->transitNodesPerCell[trgCellX][trgCellY][j];
            int distBetweenTn = tnData->distancesBetweenTransitNodes[srcTn][trgTn];
            long dist = std::max(srcToTnDist + trgToTnDist + distBetweenTn, distBetweenTn);
            minDist = std::min(dist, minDist);
        }
    }
    return minDist;
}

TnQueryResult TransitNodesQuery::query(int source, int target) {
    if (lessThanNGridCellsAway(source, target, 4))
        return TnQueryResult(graph->dijkstra(source, target).distance, false);
    
    return TnQueryResult(query_alg(source, target), true);
}

// executes the query including the path
// i am convinced that the algorithm is correct
//but since distance queries are wrong sometimes this practically never returns the optimal path
std::pair<ResultDTO, bool> TransitNodesQuery::path_query(int source, int target) {
    // source and target are less than 8 grid cells apart -> run dijkstra
    if (lessThanNGridCellsAway(source, target, 8))
        return std::make_pair(graph->dijkstra(source, target), false);

    std::set<int> fwdNodes {source};
    std::set<int> bwdNodes {target};
    std::vector<int> fwdPath {source};
    std::vector<int> bwdPath {target};
    int pathLength = query_alg(source, target);
    int remLengthFwd = pathLength;
    int remLengthBwd = pathLength;
    int u_bwd = target;
    int u_fwd = source;
    bool shopaFound = false;
    while (!shopaFound) {
        bool noProgress = true;
        if (!lessThanNGridCellsAway(u_fwd, target, 4)) {
            for (int i = graph->offsets[u_fwd]; i < graph->offsets[u_fwd + 1]; i++) {
                int v = graph->targets[i];
                int cost = graph->costs[i];
                int dist = query_alg(v, target) + cost;
                if (dist == remLengthFwd) {
                    noProgress = false;
                    remLengthFwd -= cost;
                    u_fwd = v;
                    fwdNodes.insert(u_fwd);
                    fwdPath.push_back(u_fwd);
                    break;
                }
            }
        }
        if (!lessThanNGridCellsAway(u_bwd, source, 4)) {
            for (int i = graph->offsets[u_bwd]; i < graph->offsets[u_bwd + 1]; i++) {
                int v = graph->targets[i];
                int cost = graph->costs[i];
                int dist = query_alg(v, source) + cost;
                if (dist == remLengthBwd) {
                    noProgress = false;
                    remLengthBwd -= cost;
                    u_bwd = v;
                    bwdPath.push_back(u_bwd);
                    bwdNodes.insert(u_bwd);
                    break;
                }
            }
        }
        if (fwdNodes.find(u_bwd) != fwdNodes.end() || bwdNodes.find(u_fwd) != bwdNodes.end())
            shopaFound = true;
        if (noProgress) {
            pathLength = -1;
            break;
        }
    }

    std::vector<int> shopa;
    // -1 s.t. that the node in between is not included twice
    for (int i = 0; i < fwdPath.size() - 1; i++) {
        shopa.push_back(fwdPath[i]);
    }
    for (int i = bwdPath.size() - 1; i >= 0; i--) {
        shopa.push_back(bwdPath[i]);
    }
    return std::make_pair(ResultDTO(shopa, pathLength), true);
}

bool TransitNodesQuery::lessThanNGridCellsAway(int u, int v, int n) {
    int srcCellX = UtilFunctions::getCellX(graph->nodes[u], tnData->gridsize_x);
    int trgCellX = UtilFunctions::getCellX(graph->nodes[v], tnData->gridsize_x);
    int srcCellY = UtilFunctions::getCellY(graph->nodes[u], tnData->gridsize_y);
    int trgCellY = UtilFunctions::getCellY(graph->nodes[v], tnData->gridsize_y);
    return (std::abs(trgCellX - srcCellX) <= n || std::abs(trgCellX - srcCellX) >= tnData->gridsize_x - n) && std::abs(trgCellY - srcCellY) <= n;
}