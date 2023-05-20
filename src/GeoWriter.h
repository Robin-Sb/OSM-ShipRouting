#pragma once
#include <vector>
#include "Graph.h"
#include <iostream>

class GeoWriter {
    public: 
        static std::string buildPolygonGeoJson(std::vector<SingleCoast> coastlines);
        static std::string buildNodesGeoJson(std::vector<Node> nodes);
};