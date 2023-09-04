#include "GraphGenerator.h"


void GraphGenerator::generate(int n, std::vector<SingleCoast> &coastlines, float minLat, float maxLat, float minLon, float maxLon) {
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
        if (lat >= minLat && lat <= maxLat && lon >= minLon && lon <= maxLon) {
            Vec2Sphere node = Vec2Sphere(lat, lon);
            allNodes.push_back(node);
        }
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


// void GraphGenerator::generate(int n, std::vector<SingleCoast> &coastlines) {
//     generate(n, coastlines, -90, 90, -180, 180);
// }


void GraphGenerator::performPolyTestsConcurrent(std::vector<Vec2Sphere> &allNodes, InPolyTest &polyTest, int n_threads) {
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

void GraphGenerator::performEdgeSearchConcurrent(int n_threads) {
    std::vector<std::thread> threadPool;
    std::shared_ptr<std::vector<Vec2Sphere>> _nodes = std::make_shared<std::vector<Vec2Sphere>>(nodes);
    SphericalGrid grid = SphericalGrid(_nodes);
    std::vector<std::shared_ptr<std::vector<Edge>>> edgesAll;
    // std::vector<std::shared_ptr<std::vector<int>>> sources_trimmed;
    // std::vector<std::shared_ptr<std::vector<int>>> targets_trimmed;
    // std::vector<std::shared_ptr<std::vector<int>>> dists_trimmed;

    for (int i = 0; i < n_threads; i++) {
        // std::vector<int> sources_partial;
        // std::vector<int> targets_partial;
        // std::vector<int> dists_partial;
        std::vector<Edge> edgesPartial;
        std::shared_ptr<std::vector<Edge>> edges_ptr = std::make_shared<std::vector<Edge>>(edgesPartial);
        // std::shared_ptr<std::vector<int>> sources_ptr = std::make_shared<std::vector<int>>(sources_partial);
        // std::shared_ptr<std::vector<int>> targets_ptr = std::make_shared<std::vector<int>>(targets_partial);
        // std::shared_ptr<std::vector<int>> dists_ptr = std::make_shared<std::vector<int>>(dists_partial);
        int startIndex = std::floor(i * (nodes.size() / n_threads));
        int endIndex = std::floor((i + 1) * (nodes.size() / n_threads));
        if (i == nodes.size() - 1) 
            endIndex = nodes.size();
        threadPool.push_back(std::thread(findEdgeConcurrent, std::ref(nodes), startIndex, endIndex, std::ref(grid), edges_ptr));
        edgesAll.push_back(edges_ptr);
        // targets_trimmed.push_back(targets_ptr);
        // dists_trimmed.push_back(dists_ptr);
    }

    std::vector<Edge> edges;
    for (int i = 0; i < n_threads; i++) {
        threadPool[i].join();
        for (int j = 0; j < edgesAll[i]->size(); j++) {
            edges.push_back(edgesAll[i]->at(j));
            // sources.push_back(sources_trimmed[i]->at(j));
            // targets.push_back(targets_trimmed[i]->at(j));
            // costs.push_back(dists_trimmed[i]->at(j));
        }
    }
    std::sort(edges.begin(), edges.end(), [](Edge const& edge1, Edge const& edge2) {return edge1.source < edge2.source;});
    // bad quadratic function which filters duplicates
    for (int i = 0; i < edges.size(); i++) {
        bool existsAlready = false;
        for (int j = 0; j < i; j++) {
            if (edges[i].source == edges[j].source && edges[i].target == edges[j].target) {
                existsAlready = true;
            }
        }
        if (!existsAlready) {
            sources.push_back(edges[i].source);
            targets.push_back(edges[i].target);
            costs.push_back(edges[i].cost);
        }
    }
    // auto permu = sort_permutation(sources, [](int const& a, int const& b) {return a < b;});
    // sources = apply_permutation(sources, permu);
    // targets = apply_permutation(targets, permu);
    // costs = apply_permutation(costs, permu);
}

void GraphGenerator::addNodeConcurrent(std::vector<Vec2Sphere> &allNodes, int rangeStart, int rangeEnd, InPolyTest &polyTest, std::shared_ptr<std::vector<Vec2Sphere>> outNodes) {
    for (int i = rangeStart; i < rangeEnd; i++) {
        // brute force antarctica case because coastline only goes to -85.0511
        if (allNodes[i].lat < -85)
            continue;
        if (!polyTest.performPointInPolyTest(allNodes[i])) {
            outNodes->push_back(allNodes[i]);
        }
    }
}

void GraphGenerator::findEdgeConcurrent(std::vector<Vec2Sphere> &allNodes, int startIndex, int endIndex, SphericalGrid &grid,  std::shared_ptr<std::vector<Edge>> _edges) {
    for (int i = startIndex; i < endIndex; i++) {
        FoundNodes closestPoints = grid.findClosestPoints(allNodes[i], 30000);
        if (closestPoints.leftBottom.index != -1) {
            _edges->push_back(Edge(i, closestPoints.leftBottom.index, std::floor(closestPoints.leftBottom.dist)));
            _edges->push_back(Edge(closestPoints.leftBottom.index, i, std::floor(closestPoints.leftBottom.dist)));
            // _sources->push_back(i);
            // _targets->push_back(closestPoints.leftBottom.index);
            // _costs->push_back(std::floor(closestPoints.leftBottom.dist));
        }
        if (closestPoints.rightBottom.index != -1) {
            _edges->push_back(Edge(i, closestPoints.rightBottom.index, std::floor(closestPoints.rightBottom.dist)));
            _edges->push_back(Edge(closestPoints.rightBottom.index, i, std::floor(closestPoints.rightBottom.dist)));
            // _sources->push_back(i);
            // _targets->push_back(closestPoints.rightBottom.index);
            // _costs->push_back(std::floor(closestPoints.rightBottom.dist));
        }
        if (closestPoints.leftTop.index != -1) {
            _edges->push_back(Edge(i, closestPoints.leftTop.index, std::floor(closestPoints.leftTop.dist)));
            _edges->push_back(Edge(closestPoints.leftTop.index, i, std::floor(closestPoints.leftTop.dist)));
            // _sources->push_back(i);
            // _targets->push_back(closestPoints.leftTop.index);
            // _costs->push_back(std::floor(closestPoints.leftTop.dist));
        }
        if (closestPoints.rightTop.index != -1) {
            _edges->push_back(Edge(i, closestPoints.rightTop.index, std::floor(closestPoints.rightTop.dist)));
            _edges->push_back(Edge(closestPoints.rightTop.index, i, std::floor(closestPoints.rightTop.dist)));
            // _sources->push_back(i);
            // _targets->push_back(closestPoints.rightTop.index);
            // _costs->push_back(std::floor(closestPoints.rightTop.dist));
        }
    }
}