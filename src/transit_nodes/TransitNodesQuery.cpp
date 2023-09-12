#include "TransitNodesQuery.h"

TransitNodesQuery::TransitNodesQuery(std::shared_ptr<Graph> _graph, TransitNodesData _tnData) {
    graph = _graph;
    tnData = _tnData;
}

int TransitNodesQuery::query(int source, int target) {
    int srcCellX = UtilFunctions::getCellX(graph->nodes[source], tnData.gridsize_x);
    int trgCellX = UtilFunctions::getCellX(graph->nodes[target], tnData.gridsize_x);
    int srcCellY = UtilFunctions::getCellY(graph->nodes[source], tnData.gridsize_y);
    int trgCellY = UtilFunctions::getCellY(graph->nodes[target], tnData.gridsize_y);
    if ((std::abs(trgCellX - srcCellX) <= 4 || std::abs(trgCellX - srcCellX) >= tnData.gridsize_x - 4) && std::abs(trgCellY - srcCellY) <= 4)
        return graph->dijkstra(source, target).distance;


    int minDist = 2000000000;
    for (int i = 0; i < tnData.distancesToLocalTransitNodes[source].size(); i++) {
        for (int j = 0; j < tnData.distancesToLocalTransitNodes[target].size(); j++) {
            int srcToTnDist = tnData.distancesToLocalTransitNodes[source][i];
            int trgToTnDist = tnData.distancesToLocalTransitNodes[target][j];
            int srcTn = tnData.transitNodesPerCell[srcCellX][srcCellY][i];
            int trgTn = tnData.transitNodesPerCell[trgCellX][trgCellY][j];
            int distBetweenTn = tnData.distancesBetweenTransitNodes[srcTn][trgTn];
            int dist = srcToTnDist + trgToTnDist + distBetweenTn;
            // if (srcToTnDist != graph->dijkstra(tnData.transitNodes[srcTn], source).distance || trgToTnDist != graph->dijkstra(target, tnData.transitNodes[trgTn]).distance || graph->dijkstra(tnData.transitNodes[srcTn], tnData.transitNodes[trgTn]).distance != distBetweenTn)
            //     int x = 3;
            minDist = std::min(dist, minDist);
        }
    }
    return minDist;
}