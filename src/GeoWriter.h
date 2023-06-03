#pragma once
#include <vector>
#include "Graph.h"
#include <iostream>
#include <fstream>


class GeoWriter {
    public: 
        static std::string buildPolygonGeoJson(std::vector<SingleCoast> &coastlines);
        static std::string buildNodesGeoJson(std::vector<Vec2Sphere> &nodes);
        static std::string buildGraphGeoJson(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, std::vector<Vec2Sphere> &drawNodes);
        static std::string buildPathGeoJson(ResultDTO &path);
        static void writeToDisk(std::string j_string, std::string file_name);
        static std::string generateFMI(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, std::vector<int> &costs);
};