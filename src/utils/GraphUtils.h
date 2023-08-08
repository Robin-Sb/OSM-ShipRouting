#pragma once
#include <vector>
#include "Utils.h"

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


struct SearchResult {
    SearchResult() {}    
    SearchResult(int _index, float _dist) {
        index = _index;
        dist = _dist;
    }
    int index;
    float dist;
};

struct FoundNodes {
    SearchResult leftBottom;
    SearchResult rightBottom;
    SearchResult leftTop;
    SearchResult rightTop;
};

class SphericalGrid {
    public:
        SphericalGrid(std::shared_ptr<std::vector<Vec2Sphere>> _nodes);
        void addPoint(int nodeIndex, Vec2Sphere loc);
        std::vector<int> getPointsAt(Vec2Sphere loc);
        FoundNodes findClosestPoints(Vec2Sphere loc, int range);
        int findClosestPoint(Vec2Sphere pos);
    private:
        std::shared_ptr<std::vector<Vec2Sphere>> nodes;
        std::array<std::array<std::vector<int>, 180>, 360> cells;
        int getIndexLat(Vec2Sphere loc);
        int getIndexLon(Vec2Sphere loc);
};

enum SearchDirection {
    LEFT_BOTTOM,
    LEFT_TOP,
    RIGHT_TOP,
    RIGHT_BOTTOM
};

class CellSearch {
    public:
        CellSearch(int _range, Vec2Sphere _loc, std::shared_ptr<std::array<std::array<std::vector<int>, 180>, 360>> _cells, 
        std::shared_ptr<std::vector<Vec2Sphere>> _nodes, int _startIdxLat, int _startIdxLon): 
            cells(_cells), 
            range(_range), 
            loc(_loc),
            nodes(_nodes),
            startIdxLat(_startIdxLat),
            startIdxLon(_startIdxLon) {};
        FoundNodes startSearch();
    private:
        int range;
        Vec2Sphere loc;
        int startIdxLat;
        int startIdxLon;
        std::shared_ptr<std::vector<Vec2Sphere>> nodes; 
        std::shared_ptr<std::array<std::array<std::vector<int>, 180>, 360>> cells;
        SearchResult expandSearch(int idxLat, int idxLon, int dirLat, int dirLon, SearchDirection searchDir, int iterCount);
        bool checkLeft(Vec2Sphere ref, Vec2Sphere comp);
        bool checkTop(Vec2Sphere ref, Vec2Sphere comp);
        bool checkRight(Vec2Sphere ref, Vec2Sphere comp);
        bool checkBottom(Vec2Sphere ref, Vec2Sphere comp);
        SearchResult updateResult(SearchResult searchResult, SearchDirection searchDir, Vec2Sphere ref, Vec2Sphere comp, int value);
        Vec2Sphere getBoundaryCoords(int idxLat, int idxLon, int dirLat, int dirLon);
};

class GraphBuilder {
    public:
        static void buildGraphFromFMI(std::string filename);
        static void readNodes();
        static void readEdges();
};
