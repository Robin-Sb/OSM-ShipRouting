#include "TransitNodesEvaluation.h"
#

TransitNodesEvaluation::TransitNodesEvaluation(std::shared_ptr<Graph> _graph, std::shared_ptr<TransitNodesData> _tnData, int _gridsize) {
    tnData = _tnData;
    graph = _graph;
    gridsize = _gridsize;
}

void TransitNodesEvaluation::benchmark() {
    tnStats();
    //speedBenchmark();
}

void TransitNodesEvaluation::speedBenchmark() {
    TransitNodesQuery tnQuery = TransitNodesQuery(graph, tnData);
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, graph->nodes.size() - 1); // define the range
    int n = 1000;
    auto now = std::chrono::high_resolution_clock::now();
    double total_time_dijkstra = 0;
    double total_time_tn = 0;
    double total_time_long_tn = 0;
    double total_time_long_dijkstra = 0;
    double total_time_short_tn = 0;
    double total_time_short_dijkstra = 0;
    int n_short_queries;
    int n_long_queries;
    for(int i = 0; i < n; ++i) {
        int source = distr(gen);
        int target = distr(gen);
        auto startTn = std::chrono::high_resolution_clock::now();
        TnQueryResult resultTn = tnQuery.query(source, target);
        auto endTn = std::chrono::high_resolution_clock::now();
        double duration_tn = std::chrono::duration_cast<std::chrono::nanoseconds> (endTn-startTn).count();

        auto startDijkstra = std::chrono::high_resolution_clock::now();
        int resultDijkstra = graph->dijkstra(source, target).distance;
        auto endDijkstra = std::chrono::high_resolution_clock::now();
        double duration_dijkstra = std::chrono::duration_cast<std::chrono::nanoseconds> (endDijkstra-startDijkstra).count();

        if (resultTn.distance != resultDijkstra) {
            std::cout << "result wrong for " << source << ", " << target << "\n";
            continue;
        }

        total_time_dijkstra += duration_dijkstra;
        total_time_tn += duration_tn;
        if (resultTn.long_range) {
            n_long_queries++;
            total_time_long_dijkstra += duration_dijkstra; 
            total_time_long_tn += duration_tn;
        } else {
            n_short_queries++;
            total_time_short_dijkstra += duration_dijkstra;
            total_time_short_tn += duration_tn;
        }
    }
    double dijkstra_total_ms = total_time_dijkstra / 1000.0;
    double dijkstra_short_ms = total_time_short_dijkstra / 1000.0;
    double dijkstra_long_ms = total_time_long_dijkstra / 1000.0;

    double tn_total_ms = total_time_tn / 1000.0;
    double tn_short_ms = total_time_short_tn / 1000.0;
    double tn_long_ms = total_time_long_tn / 1000.0;
    std::string timeEval;
    timeEval + "dijkstra: " + std::to_string(dijkstra_total_ms) + "ms\n";
    timeEval + "tn: " + std::to_string(tn_total_ms) + "ms\n";
    timeEval + "dijkstra short range: " + std::to_string(dijkstra_short_ms) + "ms\n";
    timeEval + "tn short range: " + std::to_string(tn_short_ms) + "ms\n";
    timeEval + "dijkstra long range: " + std::to_string(dijkstra_long_ms) + "ms\n";
    timeEval + "tn long range: " + std::to_string(tn_long_ms) + "ms\n";
    timeEval + "dijkstra  avg: "  + std::to_string(dijkstra_total_ms / static_cast<double>(n)) + "ms\n";
    timeEval + "tn avg: " + std::to_string(tn_total_ms / static_cast<double>(n)) + "ms\n";
    timeEval + "dijkstra short range avg: " + std::to_string(dijkstra_short_ms / static_cast<double>(n_short_queries)) + "ms\n";
    timeEval + "tn short range avg: " + std::to_string(tn_short_ms / static_cast<double>(n_short_queries)) + "ms\n";
    timeEval + "dijkstra long range avg: " + std::to_string(dijkstra_long_ms / static_cast<double>(n_long_queries)) + "ms\n";
    timeEval + "tn long range avg: " + std::to_string(tn_long_ms / static_cast<double>(n_long_queries)) + "ms\n";
    timeEval + "# of long range queries: " + std::to_string(n_long_queries) + "\n";
    timeEval + "# of short range queries: " + std::to_string(n_short_queries) + "\n";

    GeoWriter::writeToDisk(timeEval, "../eval/speed_benchmark.txt");
}

void TransitNodesEvaluation::tnStats() {
    int totalCount = 0;
    int n_nonEmptyCells = 0;
    int maxCellX = 0;
    int maxCellY = 0;
    int maxTns = 0;
    int minTns = 200000000;
    for (int i = 0; i < tnData->transitNodesPerCell.size(); i++) {
        for (int j = 0; j < tnData->transitNodesPerCell[i].size(); j++) {
            int n_tns = tnData->transitNodesPerCell[i][j].size();
            if (n_tns > maxTns) {
                maxTns = n_tns;
                maxCellX = i;
                maxCellY = j;
            }
            maxTns = std::max(n_tns, maxTns);
            totalCount += n_tns; 
            if (n_tns != 0) {
                n_nonEmptyCells++;
                minTns = std::min(n_tns, minTns);
            }
        }
    }
    std::string tnEvalResults;

    tnEvalResults += "gridsize x: " + std::to_string(tnData->gridsize_x) + ", gridsize y: " + std::to_string(tnData->gridsize_y) + "\n";
    tnEvalResults += "# transit nodes: " + std::to_string(tnData->transitNodes.size()) + "\n";
    tnEvalResults += "max transit nodes per cell: " + std::to_string(maxTns) + " at " + std::to_string(maxCellX) + ", " + std::to_string(maxCellY) + "\n";
    tnEvalResults += "# non empty cells: " + std::to_string(n_nonEmptyCells) + "\n";
    tnEvalResults += "min non empty cells: " + std::to_string(minTns) + "\n";
    tnEvalResults += "average tns per non empty cell " + std::to_string(static_cast<double>(totalCount) / static_cast<double>(n_nonEmptyCells)) + "\n";

    std::cout << tnEvalResults;
    GeoWriter::writeToDisk(tnEvalResults, "../eval/tn_stats.txt");

    logCell(maxCellX, maxCellY);
}

void TransitNodesEvaluation::logCell(int gridCellX, int gridCellY) {
    std::vector<Vec2Sphere> nodes;
    for (int i = 0; i < tnData->transitNodesPerCell[gridCellX][gridCellY].size(); i++) {
        nodes.push_back(graph->nodes[tnData->transitNodes[tnData->transitNodesPerCell[gridCellX][gridCellY][i]]]);
    }
    GeoWriter::buildNodesGeoJson(nodes, "../eval/cell" + std::to_string(gridCellX) + "-" + std::to_string(gridCellY) + ".json");

    int minCellX = (gridCellX + gridsize - 5) % gridsize;
    int maxCellX = (gridCellX + 5) % gridsize;
    int minCellY = std::max(-gridsize, gridCellY - 5);
    int maxCellY = std::min(gridsize - 1, gridCellY + 5);

    float minLon = UtilFunctions::unprojectX(static_cast<float>(minCellX) / static_cast<float>(gridsize));
    float maxLon = UtilFunctions::unprojectX(static_cast<float>(maxCellX) / static_cast<float>(gridsize));
    float minLat = UtilFunctions::unprojectY(static_cast<float>(minCellY) / static_cast<float>(gridsize));
    float maxLat = UtilFunctions::unprojectY(static_cast<float>(maxCellY) / static_cast<float>(gridsize));

    std::string graph_json = GeoWriter::buildGraphGeoJson(graph->nodes, graph->sources, graph->targets, minLat, maxLat, minLon, maxLon);
    GeoWriter::writeToDisk(graph_json, "../eval/graph-at" + std::to_string(gridCellX) + "-" + std::to_string(gridCellY) + ".json");
}