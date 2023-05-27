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
    }
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
    }
}

void Graph::readNodes(std::ifstream &file, int n) {
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
    }
}

void Graph::addNodeConcurrent(std::vector<Vec2Sphere> &allNodes, int rangeStart, int rangeEnd, InPolyTest &polyTest, std::shared_ptr<std::vector<Vec2Sphere>> outNodes) {
    for (int i = rangeStart; i < rangeEnd; i++) {
        if (!polyTest.performPointInPolyTest(allNodes[i])) {
            outNodes->push_back(allNodes[i]);
        }
    }
}


void Graph::generate(int n, std::vector<SingleCoast> coastlines) {
    InPolyTest polyTest = InPolyTest(coastlines);
    float pi = 3.141592;
    float r = 1;
    //std::vector<Vec2Sphere> _nodes;
    float rad_to_deg = 180 / pi;
    float goldenRatio = (1 + std::pow(5, 0.5)) / 2;
    auto start = std::chrono::system_clock::now();
    std::vector<Vec2Sphere> allNodes;
    for (int i = 0; i < n; i++) {
        float theta = fmod(pi * (1 + std::pow(5, 0.5)) * i, 2 * pi);
        float phi = std::acos(1 - 2 * (i+0.5)/n);
        float lat = phi * rad_to_deg - 90;
        float lon = theta * rad_to_deg - 180;
        Vec2Sphere node = Vec2Sphere(lat, lon);
        allNodes.push_back(node);
    }
    performPolyTestsConcurrent(allNodes, polyTest);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "finished computation at " << std::ctime(&end_time)
            << "elapsed time: " << elapsed_seconds.count() << "s"
            << std::endl;
    std::shared_ptr<std::vector<Vec2Sphere>> _nodes = std::make_shared<std::vector<Vec2Sphere>>(nodes);
    SphericalGrid grid = SphericalGrid(_nodes);
    for (int i = 0; i < nodes.size(); i++) {
        drawNodes.push_back(nodes[i]);
        FoundNodes closestPoints = grid.findClosestPoints(nodes[i], 30000);
        if (closestPoints.leftBottom.index != -1) {
            sources.push_back(i);
            targets.push_back(closestPoints.leftBottom.index);
            costs.push_back(std::floor(closestPoints.leftBottom.dist));
        }
        if (closestPoints.rightBottom.index != -1) {
            sources.push_back(i);
            targets.push_back(closestPoints.rightBottom.index);
            costs.push_back(std::floor(closestPoints.rightBottom.dist));
        }
        if (closestPoints.leftTop.index != -1) {
            sources.push_back(i);
            targets.push_back(closestPoints.leftTop.index);
            costs.push_back(std::floor(closestPoints.leftTop.dist));
        }
        if (closestPoints.rightTop.index != -1) {
            sources.push_back(i);
            targets.push_back(closestPoints.rightTop.index);
            costs.push_back(std::floor(closestPoints.rightTop.dist));
        }
    } 
}

void Graph::performPolyTestsConcurrent(std::vector<Vec2Sphere> &allNodes, InPolyTest &polyTest) {
    std::vector<std::shared_ptr<std::vector<Vec2Sphere>>> nodes_trimmed;
    std::vector<std::thread> threadPool;

    for (int i = 0; i < 6; i++) {
        std::vector<Vec2Sphere> nodes_partial;
        std::shared_ptr<std::vector<Vec2Sphere>> nodes_ptr = std::make_shared<std::vector<Vec2Sphere>>(nodes_partial);
        int startIndex = i * (allNodes.size() / 6);
        int endIndex = (i + 1) * (allNodes.size() / 6);
        threadPool.push_back(std::thread(addNodeConcurrent, std::ref(allNodes), startIndex, endIndex, std::ref(polyTest), nodes_ptr));
        nodes_trimmed.push_back(nodes_ptr);
    }

    for (int i = 0; i < 6; i++) {
        threadPool[i].join();
        for (int j = 0; j < nodes_trimmed[i]->size(); j++) {
            nodes.push_back(nodes_trimmed[i]->at(j));
        }
    }
}
