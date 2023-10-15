#pragma once
#include <random>
#include <chrono>
#include <ctime>

#include "TransitNodesDef.h"
#include "../graph/Graph.h"
#include "../io/GeoWriter.h"
#include "TransitNodesQuery.h"

class TransitNodesEvaluation {
    public:
        TransitNodesEvaluation(std::shared_ptr<Graph> _graph, std::shared_ptr<TransitNodesData> _tnData, int _gridsize); 
        void benchmark();
        void logCell(int gridCellX, int gridCellY);
        void logTns(int gridCellX, int gridCellY);
        std::vector<std::pair<int, int>> read_permutation(std::string file_name);
        void generate_permutation(int n_nodes, int n);

    private:
        std::shared_ptr<Graph> graph;
        std::shared_ptr<TransitNodesData> tnData;
        int gridsize;
        void speedBenchmark();
        void tnStats();
};