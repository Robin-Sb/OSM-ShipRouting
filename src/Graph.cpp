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
    int startIndex = sGrid->findClosestPoint(startPos);
    int endIndex = sGrid->findClosestPoint(endPos);
    std::vector<Vec2Sphere> nodePath;
    if (startIndex == -1) {
        std::cout << "No adjacent node found. Are you starting from land?" << std::endl;
        return ResultDTO(nodePath, -1);
    }
    if (endIndex == -1) {
        std::cout << "End node not found. Are you trying to travel to land?" << std::endl;
        return ResultDTO(nodePath, -1);
    }
    return dijkstra(startIndex, endIndex);
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
    std::set<int> explored;

    while (!pq.empty()) {
        std::pair<int, int> node = pq.top();
        pq.pop();
        int u = node.second;
        if (u == target) 
            break;
        explored.insert(u);
        for (int i = offsets[u]; i < offsets[u + 1]; i++) {
            int v = targets[i];
            if (explored.count(v))
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

void Graph::addNodeConcurrent(std::vector<Vec2Sphere> &allNodes, int rangeStart, int rangeEnd, InPolyTest &polyTest, std::shared_ptr<std::vector<Vec2Sphere>> outNodes) {
    for (int i = rangeStart; i < rangeEnd; i++) {
        // brute force antarctica case because coastline only goes to -85.0511
        if (allNodes[i].lat < -85)
            continue;
        if (!polyTest.performPointInPolyTest(allNodes[i])) {
            outNodes->push_back(allNodes[i]);
        }
    }
}


void Graph::generate(int n, std::vector<SingleCoast> coastlines) {

    InPolyTest polyTest = InPolyTest(coastlines);
    const float pi = 3.141592;
    const float r = 6371;
    std::random_device rand_dev_z;
    std::mt19937 generator_z(rand_dev_z());
    std::uniform_real_distribution<float> distr_z(-r, r);
    std::random_device rand_dev_phi;
    std::mt19937 generator_phi(rand_dev_phi());
    std::uniform_real_distribution<float> distr_phi(0, 2 * pi);

    float rad_to_deg = 180 / pi;
    auto start = std::chrono::system_clock::now();
    std::vector<Vec2Sphere> allNodes;
    for (int i = 0; i < n; i++) {
        float z = distr_z(generator_z);
        float phi = distr_phi(generator_phi);
        float x = std::sqrt(r * r - z * z) * std::cos(phi);
        float y = std::sqrt(r * r - z * z) * std::sin(phi);
        float lat = std::asin(z / r) * rad_to_deg;
        float lon = std::atan2(y, x) * rad_to_deg;
        Vec2Sphere node = Vec2Sphere(lat, lon);
        allNodes.push_back(node);
    }
    performPolyTestsConcurrent(allNodes, polyTest, 6);
    performEdgeSearchConcurrent(6);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "finished computation at " << std::ctime(&end_time)
            << "elapsed time: " << elapsed_seconds.count() << "s"
            << std::endl;
}

void Graph::findEdgeConcurrent(std::vector<Vec2Sphere> &allNodes, int startIndex, int endIndex, SphericalGrid &grid,  std::shared_ptr<std::vector<int>> _sources, std::shared_ptr<std::vector<int>> _targets, std::shared_ptr<std::vector<int>> _costs) {
    for (int i = startIndex; i < endIndex; i++) {
        FoundNodes closestPoints = grid.findClosestPoints(allNodes[i], 30000);
        if (closestPoints.leftBottom.index != -1) {
            _sources->push_back(i);
            _targets->push_back(closestPoints.leftBottom.index);
            _costs->push_back(std::floor(closestPoints.leftBottom.dist));
        }
        if (closestPoints.rightBottom.index != -1) {
            _sources->push_back(i);
            _targets->push_back(closestPoints.rightBottom.index);
            _costs->push_back(std::floor(closestPoints.rightBottom.dist));
        }
        if (closestPoints.leftTop.index != -1) {
            _sources->push_back(i);
            _targets->push_back(closestPoints.leftTop.index);
            _costs->push_back(std::floor(closestPoints.leftTop.dist));
        }
        if (closestPoints.rightTop.index != -1) {
            _sources->push_back(i);
            _targets->push_back(closestPoints.rightTop.index);
            _costs->push_back(std::floor(closestPoints.rightTop.dist));
        }
    }
}

void Graph::performEdgeSearchConcurrent(int n_threads) {
    std::vector<std::thread> threadPool;
    std::shared_ptr<std::vector<Vec2Sphere>> _nodes = std::make_shared<std::vector<Vec2Sphere>>(nodes);
    SphericalGrid grid = SphericalGrid(_nodes);
    std::vector<std::shared_ptr<std::vector<int>>> sources_trimmed;
    std::vector<std::shared_ptr<std::vector<int>>> targets_trimmed;
    std::vector<std::shared_ptr<std::vector<int>>> dists_trimmed;

    for (int i = 0; i < n_threads; i++) {
        std::vector<int> sources_partial;
        std::vector<int> targets_partial;
        std::vector<int> dists_partial;
        std::shared_ptr<std::vector<int>> sources_ptr = std::make_shared<std::vector<int>>(sources_partial);
        std::shared_ptr<std::vector<int>> targets_ptr = std::make_shared<std::vector<int>>(targets_partial);
        std::shared_ptr<std::vector<int>> dists_ptr = std::make_shared<std::vector<int>>(dists_partial);
        int startIndex = std::floor(i * (nodes.size() / n_threads));
        int endIndex = std::floor((i + 1) * (nodes.size() / n_threads));
        if (i == nodes.size() - 1) 
            endIndex = nodes.size();
        threadPool.push_back(std::thread(findEdgeConcurrent, std::ref(nodes), startIndex, endIndex, std::ref(grid), sources_ptr, targets_ptr, dists_ptr));
        sources_trimmed.push_back(sources_ptr);
        targets_trimmed.push_back(targets_ptr);
        dists_trimmed.push_back(dists_ptr);
    }

    for (int i = 0; i < n_threads; i++) {
        threadPool[i].join();
        for (int j = 0; j < sources_trimmed[i]->size(); j++) {
            sources.push_back(sources_trimmed[i]->at(j));
            targets.push_back(targets_trimmed[i]->at(j));
            costs.push_back(dists_trimmed[i]->at(j));
        }
    }
    auto permu = sort_permutation(sources, [](int const& a, int const& b) {return a < b;});
    sources = apply_permutation(sources, permu);
    targets = apply_permutation(targets, permu);
    costs = apply_permutation(costs, permu);
}

void Graph::performPolyTestsConcurrent(std::vector<Vec2Sphere> &allNodes, InPolyTest &polyTest, int n_threads) {
    std::vector<std::shared_ptr<std::vector<Vec2Sphere>>> nodes_trimmed;
    std::vector<std::thread> threadPool;

    for (int i = 0; i < n_threads; i++) {
        std::vector<Vec2Sphere> nodes_partial;
        std::shared_ptr<std::vector<Vec2Sphere>> nodes_ptr = std::make_shared<std::vector<Vec2Sphere>>(nodes_partial);
        int startIndex = i * (allNodes.size() / n_threads);
        int endIndex = (i + 1) * (allNodes.size() / n_threads);
        if (i == nodes.size() - 1) 
            endIndex = allNodes.size();

        threadPool.push_back(std::thread(addNodeConcurrent, std::ref(allNodes), startIndex, endIndex, std::ref(polyTest), nodes_ptr));
        nodes_trimmed.push_back(nodes_ptr);
    }

    for (int i = 0; i < n_threads; i++) {
        threadPool[i].join();
        for (int j = 0; j < nodes_trimmed[i]->size(); j++) {
            nodes.push_back(nodes_trimmed[i]->at(j));
        }
    }
}
