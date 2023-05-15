#include <iostream>
#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>
#include <vector>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <PBFReader.h>
#include <string> 
#include <fstream>
#include <unordered_map>
#include "InPolyTest.h"

// The type of index used. This must match the include file above
using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

// The location handler always depends on the index type
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

// hacky function which outputs the coastline as geojson
std::string buildGeoJson(std::vector<SingleCoast> coastlines) {
    std::string out = "{ \"type\": \"FeatureCollection\"," 
    "\"features\": [{"
    "\"type\": \"Feature\","
    "\"properties\":{\"fill\": \"#006600\"},"
    "\"geometry\":{"
    "\"type\": \"MultiPolygon\","
    "\"coordinates\": [[";
    for (int i = 0; i < coastlines.size(); i++) {
        out += "[\n";
        for (int j = 0; j < coastlines[i].path.size(); j++) {
            bool appendId = false;
            if (appendId)
                out += "[\n" + std::to_string(coastlines[i].path[j].lon) + ",\n" + std::to_string(coastlines[i].path[j].lat) + ",\n" + std::to_string(coastlines[i].path[j].id) + "\n]";
            else 
                out += "[\n" + std::to_string(coastlines[i].path[j].lon) + ",\n" + std::to_string(coastlines[i].path[j].lat) + "\n]";

            if (j != coastlines[i].path.size() - 1) {
                out += ",";
            }
            out += "\n";
        }
        out += "]";
        if (i != coastlines.size() - 1) {
            out += ",";
        }
        out += "\n";
    }
    out += "]]}}]}";
    return out;
}


int findLongestCoastline(std::vector<SingleCoast> coastlines) {
    int maxLength = 0;
    int longestCLIndex = 0;
    for (int i = 0; i < coastlines.size(); i++) {
        if (coastlines[i].path.size() > maxLength) {
            maxLength = coastlines[i].path.size();
            longestCLIndex = i;
        }
    }
    return longestCLIndex;
}

bool performInPolyTest(std::vector<SingleCoast> coastlines, Vec2Sphere point) {
    bool isOutside = true;
    for (int i = 0; i < coastlines.size(); i++) {
        Location loc = InPolyTest::isPointInPolygon(coastlines[i].path, point);
        if (loc == Location::INSIDE)
            isOutside = false; 
    }
    return isOutside;
}

int main() {
    auto otypes = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;
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
    int longestCoastline = findLongestCoastline(coastlines);
    //postprocess(coastlines);
    std::vector<SingleCoast> singleLongestCoastline;
    singleLongestCoastline.push_back(coastlines[longestCoastline]);

    // Vec2Sphere ref = Vec2Sphere(-90, 0);
    // bool isOutside = performInPolyTest(coastlines, ref);
    // if (isOutside) 
    //     std::cout << "is outside" << std::endl;
    // else
    //     std::cout << "is inside" << std::endl;
    std::string coastlines_json = buildGeoJson(coastlines);
    std::ofstream json_stream;
    json_stream.open ("coastlines.json");
    json_stream << coastlines_json;
    json_stream.close();
    return 0;
}
