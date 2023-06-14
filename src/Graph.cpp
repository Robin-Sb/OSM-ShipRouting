#include "Graph.h"
#include <chrono>
#include <ctime>    

void Graph::buildFromFMI(std::string fmiFile) {
    std::ifstream file(fmiFile);
    if (file.is_open()) {
        std::string n_nodes;
        std::getline(file, n_nodes);
        std::string n_edges;
        std::getline(file, n_edges);
        readNodes(file, std::stoi(n_nodes.c_str()));
        readEdges(file, std::stoi(n_edges.c_str()));
        for (int i = 1; i < offsets.size(); i++) 
            offsets[i] += offsets[i - 1];
        std::shared_ptr<std::vector<Vec2Sphere>> nodes_ptr = std::make_shared<std::vector<Vec2Sphere>>(nodes);
        sGrid = std::make_unique<SphericalGrid>(nodes_ptr);
    }
}

ResultDTO Graph::performDijkstra(Vec2Sphere startPos, Vec2Sphere endPos) {
    // start the search with the node closest to the selected position
    auto startNodeSearch = std::chrono::system_clock::now();
    int startIndex = sGrid->findClosestPoint(startPos);
    int endIndex = sGrid->findClosestPoint(endPos);
    auto endNodeSearch = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds_search = endNodeSearch-startNodeSearch;
    std::cout << "elapsed time node search: " << elapsed_seconds_search.count() << "s" << std::endl;

    std::vector<Vec2Sphere> nodePath;
    if (startIndex == -1) {
        std::cout << "No adjacent node found. Are you starting from land?" << std::endl;
        return ResultDTO(nodePath, -1);
    }
    if (endIndex == -1) {
        std::cout << "End node not found. Are you trying to travel to land?" << std::endl;
        return ResultDTO(nodePath, -1);
    }
    auto startDijkstra = std::chrono::system_clock::now();
    ResultDTO result = dijkstra(startIndex, endIndex);
    auto endDijkstra = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds_dijkstra = endDijkstra-startDijkstra;
    std::cout << "elapsed time dijkstra: " << elapsed_seconds_dijkstra.count() << "s" << std::endl;
    return result;
}

ResultDTO Graph::dijkstra(int source, int target) {
    std::vector<int> dist;
    std::vector<int> prev;
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;
    for (int i = 0; i < nodes.size(); i++) {
        int max_int = 2147483647;
        dist.push_back(max_int);
        prev.push_back(-1);
    }
    dist[source] = 0;
    pq.push(std::make_pair(0, source));
    std::vector<bool> explored (nodes.size(), false);

    while (!pq.empty()) {
        std::pair<int, int> node = pq.top();
        pq.pop();
        int u = node.second;
        if (explored[u])
            continue;
        if (u == target) 
            break;
        explored[u] = true;
        for (int i = offsets[u]; i < offsets[u + 1]; i++) {
            int v = targets[i];
            if (explored[v])
                continue;

            int altDist = dist[u] + costs[v];
            if (altDist < dist[v]) {
                dist[v] = altDist;
                prev[v] = u;
                pq.push(std::make_pair(dist[v], v));
            }
        }
    }

    std::vector<int> path;
    int currentNode = target;
    while (currentNode != source) {
        path.push_back(currentNode);
        currentNode = prev[currentNode];
    }

    std::reverse(path.begin(), path.end());
    std::vector<Vec2Sphere> nodePath;
    for (int i= 0; i < path.size(); i++) {
        nodePath.push_back(nodes[path[i]]);
    }

    return ResultDTO(nodePath, dist[target]);
}

void Graph::readEdges(std::ifstream &file, int m) {
    for (int i = 0; i < m; i++) {
        std::string edge_text;
        std::getline(file, edge_text);
        edge_text = edge_text.c_str();
        int pos_src = edge_text.find(" ", 0);
        int pos_target = edge_text.find(" ", pos_src + 1);
        int pos_dist = edge_text.find(" ", pos_target + 1);
        int source = std::stoi(edge_text.substr(0, pos_src));
        int target = std::stoi(edge_text.substr(pos_src, pos_target - pos_src));
        int dist = std::stoi(edge_text.substr(pos_target, pos_dist - pos_target));
        sources.push_back(source);
        targets.push_back(target);
        costs.push_back(dist);
        offsets[source + 1] += 1;
    }
}

void Graph::readNodes(std::ifstream &file, int n) {
    offsets.push_back(0);
    for (int i = 0; i < n; i++) {
        std::string node_text;
        std::getline(file, node_text);
        node_text = node_text.c_str();
        int pos_idx = node_text.find(" ", 0);
        int idx = std::stoi(node_text.substr(0, pos_idx));
        int pos_lat = node_text.find(" ", pos_idx + 1);
        std::string s_lat = node_text.substr(pos_idx, pos_lat - pos_idx);
        float lat = std::stof(s_lat);
        int pos_lon = node_text.find(" ", pos_lat + 1);
        std::string s_lon = node_text.substr(pos_lat, pos_lon - pos_lat);
        float lon = std::stof(s_lon);
        Vec2Sphere node = Vec2Sphere(lat, lon);
        nodes.push_back(node);
        // prefill offset array
        offsets.push_back(0);
    }
}


void Graph::generate(int n, std::vector<SingleCoast> &coastlines) {
    GraphGenerator generator = GraphGenerator();
    generator.generate(n, coastlines);
    // kind of expensive copying
    nodes = generator.nodes;
    sources = generator.sources;
    targets = generator.targets;
    costs = generator.costs;
}