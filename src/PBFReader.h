#include <vector>
#include <osmium/io/pbf_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <iostream>
#include <osmium/handler.hpp>
#include <unordered_map>

class Node {
    public:
        Node(float _lat, float _lng, int _id);
        float lat;
        float lng;
        int id;

        bool operator == (const Node& otherNode) const {
            if (this->lat == otherNode.lat && this->lng == otherNode.lng) 
                return true;
            return false;
        }

        struct HashFunction {
            size_t operator()(const Node& node) const {
                size_t latHash = std::hash<float>()(node.lat);
                size_t lngHash = std::hash<float>()(node.lng) << 1;
                return latHash ^ lngHash;
            }
        };
    
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