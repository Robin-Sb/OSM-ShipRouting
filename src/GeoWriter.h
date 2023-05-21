#pragma once
#include <vector>
#include "Graph.h"
#include <iostream>
#include <fstream>


class GeoWriter {
    public: 
        static std::string buildPolygonGeoJson(std::vector<SingleCoast> coastlines);
        static std::string buildNodesGeoJson(std::vector<Vec2Sphere> nodes);
        static void writeToDisk(std::string j_string, std::string file_name);
};