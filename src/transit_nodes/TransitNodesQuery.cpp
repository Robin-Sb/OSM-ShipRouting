#include "TransitNodesQuery.h"

TransitNodesQuery::TransitNodesQuery(std::shared_ptr<Graph> _graph, TransitNodesData _tnData) {
    graph = _graph;
    tnData = _tnData;
}

int TransitNodesQuery::query(int source, int target) {
    int srcCellX = UtilFunctions::getCellX(graph->nodes[source], tnData.gridsize_x);
    int srcCellY = UtilFunctions::getCellY(graph->nodes[source], tnData.gridsize_y);
    int trgCellX = UtilFunctions::getCellX(graph->nodes[target], tnData.gridsize_x);
    int trgCellY = UtilFunctions::getCellY(graph->nodes[target], tnData.gridsize_y);
    float testX = Vec2::projectX(graph->nodes[1039].lon);
    float testY = Vec2::projectY(graph->nodes[1039].lat);
        // TODO: check distance between source and target and execute dijkstra if they are less than 4 gridcells away
    int minDist = 2000000000;
    for (int i = 0; i < tnData.distancesToLocalTransitNodes[source].size(); i++) {
        for (int j = 0; j < tnData.distancesToLocalTransitNodes[target].size(); j++) {
            int srcToTnDist = tnData.distancesToLocalTransitNodes[source][i];
            int trgToTnDist = tnData.distancesToLocalTransitNodes[target][j];

            std::cout << "i: " << i << ", j: " << j << "\n";
            int srcTn = tnData.transitNodesPerCell[srcCellX][srcCellY][i];
            int trgTn = tnData.transitNodesPerCell[trgCellX][trgCellY][j];
            int distBetweenTn = tnData.distancesBetweenTransitNodes[srcTn][trgTn];
            int dist = srcToTnDist + trgToTnDist + distBetweenTn;
            minDist = std::min(dist, minDist);
        }
    }
    return minDist;
}