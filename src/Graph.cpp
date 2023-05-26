#include "Graph.h"


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
        int source = std::stoi(edge_text.substr(0, pos_src));
        int target = std::stoi(edge_text.substr(pos_src, pos_target - pos_src));
        sources.push_back(source);
        targets.push_back(target);
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


void Graph::generate(int n, std::vector<SingleCoast> coastlines) {
    InPolyTest polyTest = InPolyTest(coastlines);
    float pi = 3.141592;
    float r = 1;
    //std::vector<Vec2Sphere> _nodes;
    float rad_to_deg = 180 / pi;
    float goldenRatio = (1 + std::pow(5, 0.5)) / 2;
    for (int i = 0; i < n; i++) {
        float theta = fmod(pi * (1 + std::pow(5, 0.5)) * i, 2 * pi);
        float phi = std::acos(1 - 2 * (i+0.5)/n);
        float lat = phi * rad_to_deg - 90;
        float lon = theta * rad_to_deg - 180;
        Vec2Sphere node = Vec2Sphere(lat, lon);
        nodes.push_back(node);
        // if (!polyTest.performPointInPolyTest(node)) {
        //     //grid.addPoint(nodes.size() - 1, node);
        // }
    }
    std::shared_ptr<std::vector<Vec2Sphere>> _nodes = std::make_shared<std::vector<Vec2Sphere>>(nodes);
    SphericalGrid grid = SphericalGrid(_nodes);
    for (int i = 10000; i < 20000; i++) {
        drawNodes.push_back(nodes[i]);
        FoundNodes closestPoints = grid.findClosestPoints(nodes[i], 30000);
        if (closestPoints.leftBottom != -1) {
            sources.push_back(i);
            targets.push_back(closestPoints.leftBottom);
        }
        if (closestPoints.rightBottom != -1) {
            sources.push_back(i);
            targets.push_back(closestPoints.rightBottom);
        }
        if (closestPoints.leftTop != -1) {
            sources.push_back(i);
            targets.push_back(closestPoints.leftTop);
        }
        if (closestPoints.rightTop != -1) {
            sources.push_back(i);
            targets.push_back(closestPoints.rightTop);
        }
    }
}