#pragma once
#include <vector>
#include "../graph/Graph.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
// refactor, imo this should not be included here (factor out TransitNodesData in some def file)
#include "../transit_nodes/TransitNodesRouting.h"


class GeoWriter {
    public: 
        static std::string buildPolygonGeoJson(std::vector<SingleCoast> &coastlines);
        static std::string buildNodesGeoJson(std::vector<Vec2Sphere> &nodes);
        static std::string buildGraphGeoJson(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets);
        static std::string buildGraphGeoJson(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, float minLat, float maxLat, float minLon, float maxLon);
        static std::string buildPathGeoJson(std::vector<Vec2Sphere> &path, int distance);
        static std::string buildLineSegmentsJson(std::vector<Vec2Sphere> lineSegments);
        static std::string buildNodesAsEdges(std::vector<Vec2Sphere> nodes);
        static std::string buildGridGeoJson(std::vector<std::pair<Vec2Sphere, Vec2Sphere>>);
        static void buildPolygonGeoJson(std::vector<SingleCoast> &coastlines, const std::string filename);
        static void buildNodesGeoJson(std::vector<Vec2Sphere> &nodes, const std::string filename);
        static void buildGraphGeoJson(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, const std::string filename);
        static void buildPathGeoJson(std::vector<Vec2Sphere> &path, int distance, const std::string filename);
        static void buildLineSegmentsJson(std::vector<Vec2Sphere> lineSegments, const std::string filename);
        static void buildNodesAsEdges(std::vector<Vec2Sphere> nodes, const std::string filename);
        static void buildGridGeoJson(std::vector<std::pair<Vec2Sphere, Vec2Sphere>> grid, const std::string filename);

        static void writeToDisk(std::string j_string, const std::string &file_name);
        static std::string generateFMI(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, std::vector<int> &costs);
        static void generateFMI(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, std::vector<int> &costs, const std::string filename);

        static void buildGraphFromFMI(std::string filename);
        static void readNodes(std::ifstream &file, int n, std::vector<int> &offsets, std::vector<Vec2Sphere> &nodes);
        static void readEdges(std::ifstream &file, int m, std::vector<int> &sources, std::vector<int> targets, std::vector<int> &costs, std::vector<int> &offsets);

        static void writeTransitNodes(TransitNodesData &tnr, const std::string filename);
        static TransitNodesData readTransitNodes(const std::string filename);
};