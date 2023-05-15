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

std::vector<SingleCoast> merge_coastline2(std::vector<SingleCoast> coastlines) {
    std::unordered_map<int, int> processedCoastlines;
    std::vector<SingleCoast> updatedCoastlines;
    std::vector<bool> isCLActive;

    for (int i = 0; i < coastlines.size(); i++) {
        int pathLength = coastlines[i].path.size();
        if (coastlines[i].path[0].id == -1969472073 || coastlines[i].path[coastlines[i].path.size() - 1].id == -1969472073) {
            std::cout << "halt here";
        }
        bool firstNodeExists = !(processedCoastlines.find(coastlines[i].path[0].id) == processedCoastlines.end());
        bool lastNodeExists = !(processedCoastlines.find(coastlines[i].path[pathLength - 1].id) == processedCoastlines.end());
        // standard case -> new coastline, append to the existing coastline and store the index in hashmap
        if (!firstNodeExists && !lastNodeExists) {
            // append the coastline
            updatedCoastlines.push_back(coastlines[i]);
            // manage inactive coastlines for the sandwich case
            isCLActive.push_back(false);
            // store first and list node id in hashmap
            processedCoastlines.insert(std::make_pair(coastlines[i].path[0].id, updatedCoastlines.size() - 1));
            processedCoastlines.insert(std::make_pair(coastlines[i].path[coastlines[i].path.size() - 1].id, updatedCoastlines.size() - 1));
        }
        int coastIndexEnd;
        // the first node of the currently checked coastline is already in the hashmap
        // this must be the end of another coastline -> just append
        if (firstNodeExists) {
            int coastIndex = processedCoastlines.find(coastlines[i].path[0].id)->second;
            coastIndexEnd = coastIndex;
            // just append all nodes to the new coastline
            for (int j = 1; j < coastlines[i].path.size(); j++) {
                updatedCoastlines[coastIndex].path.push_back(coastlines[i].path[j]);
            }
            // erase the last vertex from the old coastline
            processedCoastlines.erase(coastlines[i].path[0].id);
            // the new endpoint of the coastline is the last vertex of the new coastline
            processedCoastlines.insert(std::make_pair(coastlines[i].path[coastlines[i].path.size() - 1].id, coastIndex));
        }

        if (lastNodeExists) {
            int coastIndexStart = processedCoastlines.find(coastlines[i].path[coastlines[i].path.size() - 1].id)->second;
            if (firstNodeExists) { 
                // start and end are the same -> finished coastline -> do nothing
                if (coastIndexStart == coastIndexEnd) {
                    // it kind of does not matter what is erased here, none of these will be called later and they map to the same path anyways
                    processedCoastlines.erase(coastlines[i].path[0].id);
                    processedCoastlines.erase(updatedCoastlines[coastIndexEnd].path[0].id);
                    isCLActive[coastIndexStart] = true;
                    continue;
                }
                // start and end are not the same -> sandwich case
                std::vector<Node> newPath;
                for (int j = 0; j < updatedCoastlines[coastIndexEnd].path.size(); j++) {
                    newPath.push_back(updatedCoastlines[coastIndexEnd].path[j]);
                }

                for (int j = 1; j < updatedCoastlines[coastIndexStart].path.size(); j++) {
                    newPath.push_back(updatedCoastlines[coastIndexStart].path[j]);
                }

                updatedCoastlines[coastIndexStart].path = newPath;
                // since the coastline belonging to coastIndexEnd is inactive now, reset its value to coastIndexStart
                processedCoastlines.erase(coastlines[i].path[0].id);
                processedCoastlines.erase(coastlines[i].path[coastlines[i].path.size() - 1].id);
                processedCoastlines.erase(updatedCoastlines[coastIndexEnd].path[0].id);
                processedCoastlines.insert(std::make_pair(updatedCoastlines[coastIndexEnd].path[0].id, coastIndexStart));
                // disable coastIndexEnd
                // isCLActive[coastIndexStart] = true;
                // isCLActive[coastIndexEnd] = false;
            } else {
                std::vector<Node> newPath;
                for (int j = 0; j < coastlines[i].path.size(); j++) {
                    newPath.push_back(coastlines[i].path[j]);
                }

                for (int j = 1; j < updatedCoastlines[coastIndexStart].path.size(); j++) {
                    newPath.push_back(updatedCoastlines[coastIndexStart].path[j]);
                }
                processedCoastlines.erase(updatedCoastlines[coastIndexStart].path[0].id);
                updatedCoastlines[coastIndexStart].path = newPath;
                processedCoastlines.insert(std::make_pair(coastlines[i].path[0].id, coastIndexStart));
            }
        }
    }
    // filter inactive coastlines
    std::vector<SingleCoast> result;
    for (int i = 0; i < updatedCoastlines.size(); i++) {
        if (isCLActive[i])
            result.push_back(updatedCoastlines[i]);
    }
    return result;
}


std::vector<SingleCoast> merge_coastline(std::vector<SingleCoast> coastlines) {
    // Assumption: Always the last node and has always the same ID
    std::unordered_map<int, int> endnodes;
    //std::unordered_map<Node, int, Node::HashFunction> vertices;
    std::vector<SingleCoast> newCoastlines;
    std::vector<bool> isCLActive;
    for (int i = 0; i < coastlines.size(); i++) {
        int pathLength = coastlines[i].path.size();
        if (!(endnodes.find(coastlines[i].path[0].id) == endnodes.end()) && !(endnodes.find(coastlines[i].path[pathLength - 1].id) == endnodes.end())) {
            std::cout << endnodes.find(coastlines[i].path[0].id)->second << std::endl;
            std::cout << endnodes.find(coastlines[i].path[pathLength - 1].id)->second << std::endl;
        } 
        int updatedIndex = -1;
        bool wasUpdated = false;
        // hash only the first and the last node in a path
        if (!(endnodes.find(coastlines[i].path[0].id) == endnodes.end())) {
            int coastIndex = endnodes.find(coastlines[i].path[0].id)->second;
            // last vertex -> we just append the new partial coastline (- the first vertex)
            if (newCoastlines[coastIndex].path[newCoastlines[coastIndex].path.size() - 1].id == coastlines[i].path[0].id) {
                // remove the trailing id so that in the sandwich case the id is not ambiguous
                endnodes.erase(newCoastlines[coastIndex].path[newCoastlines[coastIndex].path.size() - 1].id);
                for (int j = 1; j < pathLength; j++) {
                    newCoastlines[coastIndex].path.push_back(coastlines[i].path[j]);
                }
                if (!(endnodes.find(coastlines[i].path[pathLength - 1].id) == endnodes.end())) {
                    updatedIndex = endnodes.find(coastlines[i].path[pathLength - 1].id)->second;
                }
                endnodes.insert(std::make_pair(coastlines[i].path[pathLength - 1].id, coastIndex));
                wasUpdated = true;
            }
        }
        
        // FIX: If partial coastline is sandwiched, else does not fit here
        if (!(endnodes.find(coastlines[i].path[pathLength - 1].id) == endnodes.end())) {
            int coastIndex = endnodes.find(coastlines[i].path[pathLength - 1].id)->second;

            if (newCoastlines[coastIndex].path[0].id == coastlines[i].path[pathLength - 1].id) {
                std::vector<Node> newPath;
                for (int j = 0; j < pathLength; j++) {
                    newPath.push_back(coastlines[i].path[j]);
                }

                for (int j = 1; j < newCoastlines[coastIndex].path.size(); j++) {
                    newPath.push_back(newCoastlines[coastIndex].path[j]);
                }
                if (wasUpdated) {
                    endnodes.erase(newCoastlines[updatedIndex].path[0].id);
                    endnodes.insert(std::make_pair(newCoastlines[updatedIndex].path[0].id, coastIndex));
                    isCLActive[updatedIndex] = false;
                }
                newCoastlines[coastIndex].path = newPath;
            }
            endnodes.insert(std::make_pair(coastlines[i].path[0].id, coastIndex));
        } else {
            endnodes.insert(std::make_pair(coastlines[i].path[0].id, newCoastlines.size()));
            endnodes.insert(std::make_pair(coastlines[i].path[coastlines[i].path.size() - 1].id, newCoastlines.size()));
            newCoastlines.push_back(coastlines[i]);
            isCLActive.push_back(true);
        }
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

void postprocess(std::vector<SingleCoast> coastlines) {
    int count = 0;
    for (int i = 0; i < coastlines.size(); i++) {
        if (coastlines[i].path[0].id != coastlines[i].path[coastlines[i].path.size() - 1].id) {
            count++;
            for (int j = 0; j < coastlines.size(); j++) {
                if (j != i) {
                    for (int k = 0; k < coastlines[j].path.size(); k++) {
                        if (coastlines[i].path[0].id == coastlines[j].path[k].id || coastlines[i].path[coastlines[i].path.size() - 1].id == coastlines[j].path[k].id) {
                            std::cout << "coastline could be closed" << std::endl;
                        }
                    }
                }
            }
            
        }
    }
    std::cout << count << std::endl;
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
    std::vector<SingleCoast> coastlines = merge_coastline2(handler.coastline);
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
