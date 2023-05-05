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

// The type of index used. This must match the include file above
using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

// The location handler always depends on the index type
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

std::vector<SingleCoast> merge_coastline(std::vector<SingleCoast> coastlines) {
    // Assumption: Always the last node and has always the same ID
    std::unordered_map<int, int> endnodes;
    std::unordered_map<Node, int, Node::HashFunction> vertices;
    std::vector<SingleCoast> newCoastlines;
    for (int i = 0; i < coastlines.size(); i++) {
        int pathLength = coastlines[i].path.size();
        // hash only the first and the last node in a path
        if (!(endnodes.find(coastlines[i].path[0].id) == endnodes.end())) {
            int coastIndex = endnodes.find(coastlines[i].path[0].id)->second;
            // last vertex -> we just append the new partial coastline (- the first vertex)
            if (newCoastlines[coastIndex].path[newCoastlines[coastIndex].path.size() - 1].id == coastlines[i].path[0].id) {
                for (int j = 1; j < pathLength; j++) {
                    newCoastlines[coastIndex].path.push_back(coastlines[i].path[j]);
                }
            }
            endnodes.insert(std::make_pair(coastlines[i].path[pathLength - 1].id, coastIndex));
        } else if (!(endnodes.find(coastlines[i].path[pathLength - 1].id) == endnodes.end())) {
            int coastIndex = endnodes.find(coastlines[i].path[pathLength - 1].id)->second;

            if (newCoastlines[coastIndex].path[0].id == coastlines[i].path[pathLength - 1].id) {
                std::vector<Node> newPath;
                for (int j = 0; j < pathLength; j++) {
                    newPath.push_back(coastlines[i].path[j]);
                }

                for (int j = 1; j < newCoastlines[coastIndex].path.size(); j++) {
                    newPath.push_back(newCoastlines[coastIndex].path[j]);
                }

                newCoastlines[coastIndex].path = newPath;
            }
            endnodes.insert(std::make_pair(coastlines[i].path[0].id, coastIndex));
        } else {
            endnodes.insert(std::make_pair(coastlines[i].path[0].id, newCoastlines.size()));
            endnodes.insert(std::make_pair(coastlines[i].path[coastlines[i].path.size() - 1].id, newCoastlines.size()));
            newCoastlines.push_back(coastlines[i]);
        }

        // bool newCoast = true;
        // int jointIdx = -1;
        // for (int j = 0; j < coastlines[i].path.size(); j++) {
        //     // node exists already
        //     if (!(vertices.find(coastlines[i].path[j]) == vertices.end())) {
        //         newCoast = false;
        //         jointIdx = j;
        //     } 
        // }
        // if (newCoast) {
        //     int idx = newCoastlines.size();
        //     newCoastlines.push_back(coastlines[i]);
        //     for (int j = 0; j < coastlines[i].path.size(); j++) {
        //         vertices.insert(std::make_pair(coastlines[i].path[j], idx));
        //     } 
        // } else {
        //     int idxOfCoastline = vertices.find(coastlines[i].path[jointIdx])->second;
        //     std::vector<Node> newPath;
        //     int endIdx = 0;
        //     // append the first part of the coastline until the correct vertex is found
        //     for (int j = 0; j < newCoastlines[idxOfCoastline].path.size(); j++) {
        //         if (newCoastlines[idxOfCoastline].path[j].lat == coastlines[i].path[jointIdx].lat && newCoastlines[idxOfCoastline].path[j].lng == coastlines[i].path[jointIdx].lng) {
        //             endIdx = j;
        //             break;
        //         }
        //         newPath.push_back(newCoastlines[idxOfCoastline].path[j]);
        //     }
        //     // hack the new coastline with same vertex in between
        //     for (int j = 0; j < coastlines[i].path.size(); j++) {
        //         newPath.push_back(coastlines[i].path[j]);
        //         // also append to hash table
        //         vertices.insert(std::make_pair(coastlines[i].path[j], idxOfCoastline));
        //     }
        //     // append the rest
        //     for (int j = endIdx; j < newCoastlines[idxOfCoastline].path.size(); j++) {
        //         newPath.push_back(newCoastlines[idxOfCoastline].path[j]);
        //     }
        // }
    }
    return newCoastlines;
}

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
            out += "[\n" + std::to_string(coastlines[i].path[j].lng) + ",\n" + std::to_string(coastlines[i].path[j].lat) + "\n]";
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

int main() {
    auto otypes = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;
    osmium::io::File input_file{"../files/antarctica-latest.osm.pbf"};
    osmium::io::Reader reader{input_file, otypes};
    
    index_type index;
    location_handler_type location_handler{index};
    CoastHandler handler;
    osmium::apply(reader, location_handler, handler);
    reader.close();
   std::vector<SingleCoast> coastlines = merge_coastline(handler.coastline);
    std::string coastlines_json = buildGeoJson(coastlines);
    std::ofstream json_stream;
    json_stream.open ("coastlines.json");
    json_stream << coastlines_json;
    json_stream.close();
    return 0;
}
