#pragma once
#include <vector>
#include <array>
#include "math.h"
#include <memory>
#include <iostream>

class Vec2Sphere {
    public:
        Vec2Sphere() {};
        Vec2Sphere(float lat, float lon);
        
        float lat;
        float lon;
        static float dist(Vec2Sphere ref, Vec2Sphere comp);
};

struct FoundNodes {
    // FoundNodes(int _leftBottom, int _rightBottom, int _leftTop, int _rightTop) {
    //     leftBottom = _leftBottom;
    //     rightBottom = _rightBottom;
    //     leftTop = _leftTop;
    //     rightTop = _rightTop;
    // }
    int leftBottom;
    int rightBottom;
    int leftTop;
    int rightTop;
};

class SphericalGrid {
    public:
        //SphericalGrid(std::vector<Vec2Sphere> _nodes): nodes(_nodes) {};
        SphericalGrid(std::shared_ptr<std::vector<Vec2Sphere>> _nodes);
        void addPoint(int nodeIndex, Vec2Sphere loc);
        std::vector<int> getPointsAt(Vec2Sphere loc);
        FoundNodes findClosestPoints(Vec2Sphere loc, int range);
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

struct SearchResult {
    SearchResult() {}    
    SearchResult(int _index, float _dist) {
        index = _index;
        dist = _dist;
    }
    int index;
    float dist;
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
        //SearchDirection searchDir;
        //std::vector<SearchResult> results;
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