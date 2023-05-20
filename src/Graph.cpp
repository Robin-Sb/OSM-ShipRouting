#include "Graph.h"

Node::Node(float _lat, float _lon, int _id) {
    lat = _lat;
    lon = _lon;
    id = _id;
};

void Graph::generate(int n) {
    // For random scattering, currently unused
    // std::random_device dev;
    // std::mt19937 rng_z(dev());
    // std::mt19937 rng_phi(dev());
    // std::uniform_int_distribution<std::mt19937::result_type> dist_z(0,180); 
    // std::uniform_int_distribution<std::mt19937::result_type> dist_phi(0,360); 
    float pi = 3.141592;
    float r = 1;
    std::vector<Vec2Sphere> _nodes;
    float rad_to_deg = 180 / pi;

    float goldenRatio = (1 + std::pow(5, 0.5)) / 2;
    for (int i = 0; i < n; i++) {
        float theta = fmod(pi * (1 + std::pow(5, 0.5)) * i, 2 * pi);
        float phi = std::acos(1 - 2 * (i+0.5)/n);
        float lat = phi * rad_to_deg - 90;
        float lon = theta * rad_to_deg - 180;
        Vec2Sphere node = Vec2Sphere(lat, lon);
        _nodes.push_back(node);
    }
    nodes = _nodes;
}