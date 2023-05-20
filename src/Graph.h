#include <vector>
#include <random>
#include  "VectorMath.h"

#pragma once


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


class Graph {
    public:
        std::vector<int> sources;
        std::vector<int> targets;
        std::vector<int> costs;
        std::vector<int> offsets;
        std::vector<Vec2Sphere> nodes;
        void generate(int n);
};