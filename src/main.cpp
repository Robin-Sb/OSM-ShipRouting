#include <iostream>
#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>
#include <vector>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <PBFReader.h>
#include <string> 
#include <unordered_map>
#include "InPolyTest.h"
#include <algorithm>
#include <random>
#include "GeoWriter.h"

// The type of index used. This must match the include file above
using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

// The location handler always depends on the index type
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

SingleCoast findShortCoastlineWithBigPerimeter(std::vector<SingleCoast> sortedCoastlines) {
    float maxPerimeter = 0;
    int maxIdx = -1;
    for (int i = sortedCoastlines.size() - 1; i > sortedCoastlines.size() - 100; i--) {
        float perimeter = 0;
        for (int j = 0; j < sortedCoastlines[i].path.size() - 1; j++) {
            float latDist = sortedCoastlines[i].path[j].lat - sortedCoastlines[i].path[j + 1].lat;
            float lonDist = sortedCoastlines[i].path[j].lon - sortedCoastlines[i].path[j + 1].lon;
            float edgeLength = std::sqrt(latDist * latDist + lonDist * lonDist);
            perimeter += edgeLength;
        } 

        if (perimeter>maxPerimeter) {
            maxPerimeter = perimeter;
            maxIdx = i;
        }
    }
    return sortedCoastlines[maxIdx];
}


// actually this usually does not return the longest coastline
SingleCoast findLongestCoastline(std::vector<SingleCoast> coastlines) {
    // sort in descending order
    std::sort(coastlines.begin(), coastlines.end(), [](SingleCoast &a, SingleCoast &b) {return a.path.size() > b.path.size();});
    return findShortCoastlineWithBigPerimeter(coastlines);
}

void simpleTestSuite(std::vector<SingleCoast> coastlines) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::mt19937 rng2(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distLat(0,180); // distribution in range [1, 6]
    std::uniform_int_distribution<std::mt19937::result_type> distLon(0,360); // distribution in range [1, 6]
    InPolyTest polyTest = InPolyTest(coastlines);
    for (int i = 0; i < 100; i++) {
        float latP = distLat(rng);
        float lonP = distLon(rng2);
        Vec2Sphere point = Vec2Sphere(latP - 90, lonP - 180);
        bool result = polyTest.performPointInPolyTest(point);
        std::string isOutText;
        if (result) 
            isOutText = " is inside";
        else
            isOutText = " is outside";
        std::cout << "Point p with lat: " << point.lat << " and lon: " << point.lon << isOutText << std::endl;
    }
}

int main() {
    auto otypes = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;
    //osmium::io::File input_file{"../files/planet-coastlinespbf-cleanedosm.pbf"};
    osmium::io::File input_file{"../files/antarctica-latest.osm.pbf"};
    osmium::io::Reader reader{input_file, otypes};
    
    index_type index;
    location_handler_type location_handler{index};
    CoastHandler handler;
    osmium::apply(reader, location_handler, handler);
    reader.close();
    CoastlineStitcher stitcher = CoastlineStitcher(handler.coastlines);
    std::vector<SingleCoast> coastlines = stitcher.stitchCoastlines();
    //std::vector<SingleCoast> coastlines = merge_coastlines(handler.coastlines);
    // SingleCoast longestCoastline = findLongestCoastline(coastlines);
    // std::vector<SingleCoast> singleLongestCoastline;
    // singleLongestCoastline.push_back(longestCoastline);
    Graph graph = Graph();
    //graph.generate(2000000, coastlines);
    graph.buildFromFMI("../files/graph.fmi");
    std::string graph_json = GeoWriter::buildGraphGeoJson(graph.nodes, graph.sources, graph.targets, graph.drawNodes);
    //std::string graph_fmi = GeoWriter::generateFMI(graph.nodes, graph.sources, graph.targets);
    //std::string nodes_json = GeoWriter::buildNodesGeoJson(graph.nodes);
    //GeoWriter::writeToDisk(graph_json, "nodes.json");
    GeoWriter::writeToDisk(graph_json, "../files/graph.json");
    //GeoWriter::writeToDisk(graph_fmi, "../files/graph.fmi");
    //simpleTestSuite(coastlines);

    return 0;
}
