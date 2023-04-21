#include <vector>
#include <osmium/io/pbf_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <iostream>
#include <osmium/handler.hpp>

class Node {
    public:
        Node(float _lat, float _lng, int _id);
        float lat;
        float lng;
        int id;
};

class SingleCoast {
    public:
        std::vector<Node> path;
};

class CoastHandler : public osmium::handler::Handler {
public:
    std::vector<SingleCoast> coastline; 
    void way(const osmium::Way& way);
    void node(const osmium::Node& node);
};