#pragma once
#include <vector>

class Node {
    public:
        Node() {};
        Node(float _lat, float _lon, int _id);

        float lat;
        float lon;
        int id;

        bool operator == (const Node& otherNode) const {
            if (this->lat == lat && this->lon == otherNode.lon) 
                return true;
            return false;
        }

        struct HashFunction {
            size_t operator()(const Node& node) const {
                size_t latHash = std::hash<float>()(node.lat);
                size_t lngHash = std::hash<float>()(node.lon) << 1;
                return latHash ^ lngHash;
            }
        };
    
};

class SingleCoast {
    public:
        std::vector<Node> path;
};
