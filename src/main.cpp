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

void generate_graph(Graph &graph, int amount) {
    auto otypes = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;
    osmium::io::File input_file{"../files/planet-coastlinespbf-cleanedosm.pbf"};
    //osmium::io::File input_file{"../files/antarctica-latest.osm.pbf"};
    osmium::io::Reader reader{input_file, otypes};
    
    index_type index;
    location_handler_type location_handler{index};
    CoastHandler handler;
    osmium::apply(reader, location_handler, handler);
    reader.close();
    CoastlineStitcher stitcher = CoastlineStitcher(handler.coastlines);
    std::vector<SingleCoast> coastlines = stitcher.stitchCoastlines();
    graph.generate(amount, coastlines);
    std::string graph_fmi = GeoWriter::generateFMI(graph.nodes, graph.sources, graph.targets, graph.costs);
    GeoWriter::writeToDisk(graph_fmi, "../files/graph_big.fmi");
}


int main() {
    Graph graph = Graph();
    graph.buildFromFMI("../graph.fmi");
    std::vector<Vec2Sphere> resultPath = graph.performDijkstra(Vec2Sphere(23.7259, -47.3978), Vec2Sphere(20.3564, -29.5896));
    // std::string graph_json = GeoWriter::buildGraphGeoJson(graph.nodes, graph.sources, graph.targets, graph.drawNodes);
    // std::string graph_fmi = GeoWriter::generateFMI(graph.nodes, graph.sources, graph.targets, graph.costs);
    // std::string nodes_json = GeoWriter::buildNodesGeoJson(graph.nodes);
    // GeoWriter::writeToDisk(graph_json, "../files/graph.json");
    // GeoWriter::writeToDisk(graph_fmi, "../files/graph.fmi");
    // GeoWriter::writeToDisk(nodes_json, "../files/nodes_all.json");
    std::string path_json = GeoWriter::buildPathGeoJson(resultPath);
    GeoWriter::writeToDisk(path_json, "../files/result_path.json");
    return 0;
}
