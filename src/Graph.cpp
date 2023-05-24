#include "Graph.h"

Graph::Graph(std::vector<SingleCoast> coastlines) {
    polyTest = InPolyTest(coastlines);
    
}

void Graph::generate(int n) {
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
    for (int i = 30000; i < 40000; i++) {
        drawNodes.push_back(nodes[i]);
        FoundNodes closestPoints = grid.findClosestPoints(nodes[i]);
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