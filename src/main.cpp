#include <iostream>
#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>
#include <vector>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <string> 
#include <unordered_map>
#include <algorithm>
#include <random>
#include "../libs/server_http.hpp"
#include "PBFProcessing.h"
#include "InPolyTest.h"
#include "GeoWriter.h"
#include "TransitNodesRouting.h"

// The type of index used. This must match the include file above
using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

// The location handler always depends on the index type
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

void generate_graph(Graph &graph, int amount, std::string &filename) {
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
    graph.generate(amount, coastlines);
    std::string graph_fmi = GeoWriter::generateFMI(graph.nodes, graph.sources, graph.targets, graph.costs);
    GeoWriter::writeToDisk(graph_fmi, filename);
}

int getStart(std::string &query, std::string name) {
    return query.find(name + "=") + name.size() + 1;
}

int getEnd(std::string &query, int start) {
    int param = query.find("&", start);
    if (param == -1) 
        param = query.size();
    return param;
}

void startServer(Graph & graph) {
    SimpleWeb::Server<SimpleWeb::HTTP> server;
    server.config.port = 8080;
    server.default_resource["GET"] = [&graph](std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Response> response, std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>::Request> request) {
        try {
            std::string query = request->query_string;
            int lat1IdxS = getStart(query, "lat1");
            int lat1IdxE = getEnd(query, lat1IdxS);
            int lat2IdxS = getStart(query, "lat2");
            int lat2IdxE = getEnd(query, lat2IdxS);
            int lon1IdxS = getStart(query, "lon1");
            int lon1IdxE = getEnd(query, lon1IdxS);
            int lon2IdxS = getStart(query, "lon2");
            int lon2IdxE = getEnd(query, lon2IdxS);
            float lat1 = std::stof(query.substr(lat1IdxS, lat1IdxE));
            float lat2 = std::stof(query.substr(lat2IdxS, lat2IdxE));
            float lon1 = std::stof(query.substr(lon1IdxS, lon1IdxE));
            float lon2 = std::stof(query.substr(lon2IdxS, lon2IdxE));
            ResultDTO result = graph.performDijkstraLogging(Vec2Sphere(lat1, lon1), Vec2Sphere(lat2, lon2));
            std::string path_json = GeoWriter::buildPathGeoJson(result);
            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Access-Control-Allow-Origin", "*");
            response->write(path_json, header);
        } catch(const std::exception &e) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
        }
    };

    std::promise<unsigned short> server_port;
    std::thread server_thread([&server, &server_port]() {
    // Start server
    server.start([&server_port](unsigned short port) {
        server_port.set_value(port);
        });
    });
    std::cout << "Server listening on port " << server_port.get_future().get() << std::endl;
    server_thread.join();
}

bool checkIfFileExists(std::string &fileName) {
    std::ifstream infile(fileName);
    return infile.good();
}

void graph_tests(Graph &graph) {
    graph.performDijkstraLogging(Vec2Sphere(-34.016241889667015, -96.50390625000001), Vec2Sphere(-27.68352808378776, -28.652343750000004));
    graph.performDijkstraLogging(Vec2Sphere(32.694865977875075, 161.71875000000003), Vec2Sphere(-28.459033019728057, 80.50781250000001));
    graph.performDijkstraLogging(Vec2Sphere(-0.3515602939922709, 69.60937500000001), Vec2Sphere(30.29701788337205, -48.33984375));
    graph.performDijkstraLogging(Vec2Sphere(-62.75472592723178, 177.71484375), Vec2Sphere(-66.86108230224609, -18.457031250000004));
    graph.performDijkstraLogging(Vec2Sphere(58.35563036280967, -20.917968750000004), Vec2Sphere(49.61070993807422, -24.082031250000004));
}

int main() {
    Graph graph = Graph();
    std::string filename = "../files/graph_4m.fmi";
    if (checkIfFileExists(filename)) {
        graph.buildFromFMI(filename);
    } else {
        generate_graph(graph, 4000000, filename);
    }
    std::shared_ptr<Graph> graph_ptr = std::make_shared<Graph>(graph);
    TransitNodesRouting tnr = TransitNodesRouting(graph_ptr, 128);
    tnr.findEdgeBuckets();
    std::vector<Vec2Sphere> gridNodes = tnr.transformBack();
    GeoWriter::buildNodesAsEdges(gridNodes, "../files/gridnodes.json");
    //tnr.debug();
    tnr.findTransitNodes();

    //graph_tests(graph);
    // std::string graph_json = GeoWriter::buildGraphGeoJson(graph.nodes, graph.sources, graph.targets);
    // GeoWriter::writeToDisk(graph_json, "../files/graph_fin.json");
    // std::string graph_file = "../files/graph_test.json";
    // GeoWriter::buildGraphGeoJson(graph.nodes, graph.sources, graph.targets, graph_file);
    //startServer(graph);
    //GeoWriter::writeToDisk(nodes_json, "../files/nodes_test.json");
    // std::string path_json = GeoWriter::buildPathGeoJson(resultPath);
    // GeoWriter::writeToDisk(path_json, "../files/result_path.json");
    return 0;
}
