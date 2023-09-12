#include "GeoWriter.h"

std::string GeoWriter::buildPolygonGeoJson(std::vector<SingleCoast> &coastlines) {
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

std::string GeoWriter::buildNodesGeoJson(std::vector<Vec2Sphere> &nodes) {
    std::string out = "{ \"type\": \"FeatureCollection\"," 
    "   \"features\": [{"
    "       \"type\": \"Feature\","
    "       \"properties\":{\"marker-color\": \"#006600\"},"
    "   \"geometry\":{"
    "       \"type\": \"MultiPoint\","
    "       \"coordinates\": [";
    for (int i = 0; i < nodes.size(); i++) {
        out += "[" + std::to_string(nodes[i].lon) + ",\n" + std::to_string(nodes[i].lat) + "\n]";

        if (i != nodes.size() - 1)
            out+=",";
        out += "\n";
    }
    out += "]}}]}";
    return out;
}

std::string GeoWriter::buildPathGeoJson(ResultDTO &result) {
    std::vector<Vec2Sphere> &path = result.path;
    std::string out = "{"
    "   \"type\": \"Feature\",\n"
    "   \"properties\": {\"stroke\": \"#DD0000\"},\n"
    "   \"geometry\": {\n"
    "       \"type\": \"LineString\",\n"
    "       \"coordinates\": [\n";
    for (int i = 0; i < path.size(); i++) {
        out += "            [" + std::to_string(path[i].lon) + ", " + std::to_string(path[i].lat) + "]";
        if (i != path.size() - 1) 
            out += ",\n";
    }
    out += "\n]\n}, \"distance\":" + std::to_string(result.distance); 
    out += "\n}";
    return out;
}

std::string GeoWriter::buildLineSegmentsJson(std::vector<Vec2Sphere> lineSegments) {
    std::string out = "{\"type\": \"FeatureCollection\",\n"
    "\"features\": [ \n";
    for (int i = 0; i < lineSegments.size(); i+=2) {
        out += "{";
        out += "    \"type\": \"Feature\",\n"
        "    \"geometry\": {\n"
        "       \"type\": \"LineString\",\n"
        "       \"coordinates\": [\n";
        out += "[" + std::to_string(lineSegments[i].lon) + "," + std::to_string(lineSegments[i].lat) + "],\n";
        out += "[" + std::to_string(lineSegments[i+1].lon) + "," + std::to_string(lineSegments[i+1].lat) + "]\n";
        out += "]}, \"properties\": {\"stroke\": \"#ff0000\", \"stroke-width\": 10}}";
        if (i < lineSegments.size() - 2) {
            out += ",";
        }
    }
    out += "]}";
    return out;
}


std::string GeoWriter::buildGraphGeoJson(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets) {
    std::string out = "{\"type\": \"FeatureCollection\",\n"
    "\"features\":[ \n";
    int startIndex = 0; std::floor(sources.size() / 2) - 50000;
    int endIndex = sources.size(); std::floor(sources.size() / 2) + 50000;
    for (int i = startIndex; i < endIndex; i++) {
        float tLonSrc = nodes[sources[i]].lon;
        float tLonTrg = nodes[targets[i]].lon;
        // if (nodes[sources[i]].lon < -175 && nodes[targets[i]].lon > 175) 
        //     tLonSrc = 180 + (180 + nodes[sources[i]].lon);
        // if (nodes[sources[i]].lon > 175 && nodes[targets[i]].lon < -175)
        //     tLonTrg = 180 + (180 + nodes[targets[i]].lon);

        // if (nodes[sources[i]].lat < 50 || nodes[sources[i]].lat > 60)
        //     continue;
        // if (tLonSrc < -15 || tLonSrc > 3) 
        //     continue; 
        out += "{";
        out += "    \"type\": \"Feature\",\n"
        "    \"geometry\": {\n"
        "       \"type\": \"LineString\",\n"
        "       \"coordinates\": [\n";
        out += "[" + std::to_string(tLonSrc) + "," + std::to_string(nodes[sources[i]].lat) + "],\n";
        out += "[" + std::to_string(tLonTrg) + "," + std::to_string(nodes[targets[i]].lat) + "]\n";
        out += "]}, \"properties\": {}}";
        if (i < endIndex - 1) {
            out += ",";
        }
    }
    // bool drawPoints = false;
    // if (drawPoints) {
    //     out += ",";
    //     for (int i = 0; i < drawNodes.size(); i++) {
    //         out += "{";
    //         out += "    \"type\": \"Feature\",\n"
    //         "    \"geometry\": {\n"
    //         "       \"type\": \"Point\",\n"
    //         "       \"coordinates\": \n";
    //         out += "[" + std::to_string(drawNodes[i].lon) + "," + std::to_string(drawNodes[i].lat) + "]\n";
    //         out += "}, \"properties\": {}}";
    //         if (i < drawNodes.size() - 1) {
    //             out += ",";
    //         }
    //     }
    // }

    out += "]}";
    return out;
}

std::string GeoWriter::buildGridGeoJson(std::vector<std::pair<Vec2Sphere, Vec2Sphere>> grid) {
    std::string out = "{\"type\": \"FeatureCollection\",\n"
    "\"features\":[ \n";
    for (int i = 0; i < grid.size(); i++) {
        out += "{\"type\": \"Feature\",\n";
        out += "\"properties\": {\"stroke\": \"#00DD00\"},\n";
        out += "\"geometry\": {\"type\": \"LineString\",\n";
        out += "\"coordinates\": [\n";
        out += "[" + std::to_string(grid[i].first.lon) + ", " + std::to_string(grid[i].first.lat) + "], \n";
        out += "[" + std::to_string(grid[i].second.lon) + ", " + std::to_string(grid[i].second.lat) + "] \n";
        out += "]}}";
        if (i < grid.size() - 1) 
            out += ",";
    }
    out += "]}";
    return out;
}

std::string GeoWriter::buildNodesAsEdges(std::vector<Vec2Sphere> nodes) {
    std::string out = "{\"type\": \"FeatureCollection\",\n"
    "\"features\":[ \n";
    for (int i = 0; i < nodes.size(); i++) {
        out += "{\"type\": \"Feature\",\n";
        out += "\"properties\": {},\n";
        out += "\"geometry\": {\"type\": \"LineString\",\n";
        out += "\"coordinates\": [\n";
        out += "[" + std::to_string(nodes[i].lon) + ", " + std::to_string(nodes[i].lat) + "], \n";
        out += "[" + std::to_string(nodes[i].lon + 0.005) + ", " + std::to_string(nodes[i].lat + 0.005) + "] \n";
        out += "]}}";
        if (i < nodes.size() - 1) 
            out += ",";
    }
    out += "]}";
    return out;
}

void GeoWriter::buildGridGeoJson(std::vector<std::pair<Vec2Sphere, Vec2Sphere>> grid, std::string filename) {
    std::string content = buildGridGeoJson(grid);
    writeToDisk(content, filename);
}

void GeoWriter::buildNodesAsEdges(std::vector<Vec2Sphere> nodes, std::string filename) {
    std::string content = buildNodesAsEdges(nodes);
    writeToDisk(content, filename);
}

void GeoWriter::buildPolygonGeoJson(std::vector<SingleCoast> &coastlines, std::string filename) {
    std::string content = buildPolygonGeoJson(coastlines);
    writeToDisk(content, filename);
}

void GeoWriter::buildNodesGeoJson(std::vector<Vec2Sphere> &nodes, std::string filename) {
    std::string content = buildNodesGeoJson(nodes);
    writeToDisk(content, filename);
}

void GeoWriter::buildGraphGeoJson(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, std::string filename) {
    std::string content = buildGraphGeoJson(nodes, sources, targets);
    writeToDisk(content, filename);

}
void GeoWriter::buildPathGeoJson(ResultDTO &path, std::string filename) {
    std::string content = buildPathGeoJson(path);
    writeToDisk(content, filename);
}
void GeoWriter::buildLineSegmentsJson(std::vector<Vec2Sphere> lineSegments, std::string filename) {
    std::string content = buildLineSegmentsJson(lineSegments);
    writeToDisk(content, filename);
}

std::string GeoWriter::generateFMI(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, std::vector<int> &costs) {
    std::string out = std::to_string(nodes.size()) + "\n";
    out += std::to_string(sources.size()) + "\n";
    for (int i = 0; i < nodes.size(); i++) {
        out += std::to_string(i) + " " + std::to_string(nodes[i].lat) + " " + std::to_string(nodes[i].lon) + "\n";
    }

    for (int i = 0; i < sources.size(); i++) {
        out += std::to_string(sources[i]) + " " + std::to_string(targets[i]) + " " + std::to_string(costs[i]) + "\n";
    }
    return out;
}

void GeoWriter::generateFMI(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, std::vector<int> &costs, std::string filename) {
    std::string result = generateFMI(nodes, sources, targets, costs);
    writeToDisk(result, filename);
}

void GeoWriter::writeToDisk(std::string j_string, std::string &file_name) {
    std::ofstream json_stream;
    json_stream.open (file_name);
    json_stream << j_string;
    json_stream.close();
}

// kind of discontinued -> maybe refactor later
void GeoWriter::buildGraphFromFMI(std::string filename) {
    std::vector<int> sources;
    std::vector<int> targets;
    std::vector<int> costs;
    std::vector<int> offsets;
    std::vector<Vec2Sphere> nodes;

    std::ifstream file(filename);
    if (file.is_open()) {
        std::string n_nodes;
        std::getline(file, n_nodes);
        std::string n_edges;
        std::getline(file, n_edges);
        readNodes(file, std::stoi(n_nodes.c_str()), offsets, nodes);
        readEdges(file, std::stoi(n_edges.c_str()), sources, targets, costs, offsets);
        for (int i = 1; i < offsets.size(); i++) 
            offsets[i] += offsets[i - 1];
        // those two will be called in graph function
        // std::shared_ptr<std::vector<Vec2Sphere>> nodes_ptr = std::make_shared<std::vector<Vec2Sphere>>(nodes);
        // sGrid = std::make_unique<SphericalGrid>(nodes_ptr);
    }
}

void GeoWriter::readNodes(std::ifstream &file, int n, std::vector<int> &offsets, std::vector<Vec2Sphere> &nodes) {
    offsets.push_back(0);
    for (int i = 0; i < n; i++) {
        std::string node_text;
        std::getline(file, node_text);
        node_text = node_text.c_str();
        int pos_idx = node_text.find(" ", 0);
        int idx = std::stoi(node_text.substr(0, pos_idx));
        int pos_lat = node_text.find(" ", pos_idx + 1);
        std::string s_lat = node_text.substr(pos_idx, pos_lat - pos_idx);
        float lat = std::stof(s_lat);
        int pos_lon = node_text.find(" ", pos_lat + 1);
        std::string s_lon = node_text.substr(pos_lat, pos_lon - pos_lat);
        float lon = std::stof(s_lon);
        Vec2Sphere node = Vec2Sphere(lat, lon);
        nodes.push_back(node);
        // prefill offset array
        offsets.push_back(0);
    }
}

void GeoWriter::readEdges(std::ifstream &file, int m, std::vector<int> &sources, std::vector<int> targets, std::vector<int> &costs, std::vector<int> &offsets) {
    for (int i = 0; i < m; i++) {
        std::string edge_text;
        std::getline(file, edge_text);
        edge_text = edge_text.c_str();
        int pos_src = edge_text.find(" ", 0);
        int pos_target = edge_text.find(" ", pos_src + 1);
        int pos_dist = edge_text.find(" ", pos_target + 1);
        int source = std::stoi(edge_text.substr(0, pos_src));
        int target = std::stoi(edge_text.substr(pos_src, pos_target - pos_src));
        int dist = std::stoi(edge_text.substr(pos_target, pos_dist - pos_target));
        sources.push_back(source);
        targets.push_back(target);
        costs.push_back(dist);
        offsets[source + 1] += 1;
    }
}


void GeoWriter::writeTransitNodes(TransitNodesData &tnr, std::string filename) {
    std::string out;
    // append #transitnodes and gridsize and #nodes
    out += std::to_string(tnr.transitNodes.size()) + "\n";
    out += std::to_string(tnr.transitNodesPerCell.size()) + "\n";
    out += std::to_string(tnr.distancesToLocalTransitNodes.size()) + "\n";
    // append transit nodes
    for (int i = 0; i < tnr.transitNodes.size(); i++) 
        out += std::to_string(tnr.transitNodes[i]) + "\n";

    for (int i = 0; i < tnr.distancesBetweenTransitNodes.size(); i++) {
        for (int j = 0; j < tnr.distancesBetweenTransitNodes[i].size(); j++) {
            out += std::to_string(tnr.distancesBetweenTransitNodes[i][j]) + "\n";
        }
    }

    for (int x = 0; x < tnr.transitNodesPerCell.size(); x++) {
        for (int y = 0; y < tnr.transitNodesPerCell[x].size(); y++) {
            for (int k = 0; k < tnr.transitNodesPerCell[x][y].size(); k++) {
                out += std::to_string(tnr.transitNodesPerCell[x][y][k])+ + "\n";
            }
            out += "-\n";
        }
    }

    for (int i = 0; i < tnr.distancesToLocalTransitNodes.size(); i++) {
        for (int k = 0; k < tnr.distancesToLocalTransitNodes[i].size(); k++) {
            out += std::to_string(tnr.distancesToLocalTransitNodes[i][k])+ + "\n";
        }
        out += "-\n";
    }
    
    GeoWriter::writeToDisk(out, filename);
}

TransitNodesData GeoWriter::readTransitNodes(std::string filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string n_tnNodes_str;
        std::getline(file, n_tnNodes_str);
        std::string gridsize_str;
        std::getline(file, gridsize_str);
        std::string n_nodes_str;
        std::getline(file, n_nodes_str);

        int n_tnNodes = std::stoi(n_tnNodes_str);
        int gridsize = std::stoi(gridsize_str);
        int n_nodes = std::stoi(n_nodes_str);
        
        std::vector<std::vector<int>> distancesBetweenTransitNodes(n_tnNodes, std::vector<int>(n_tnNodes));
        std::vector<int> transitNodes;
        std::vector<std::vector<std::vector<int>>> transitNodesPerCell(gridsize, std::vector<std::vector<int>>(gridsize));
        std::vector<std::vector<int>> distancesToLocalTransitNodes(n_nodes);
        for (int i = 0; i < n_tnNodes; i++) {
            std::string line;
            std::getline(file, line);
            transitNodes.push_back(std::stoi(line));
        }
        for (int i = 0; i < n_tnNodes; i++) {
            for (int j = 0; j < n_tnNodes; j++) {
                std::string line;
                std::getline(file, line);
                distancesBetweenTransitNodes[i][j] = std::stoi(line);
            }
        }

        for (int x = 0; x < gridsize; x++) {
            for (int y = 0; y < gridsize; y++) {
                while (true) {
                    std::string line;
                    std::getline(file, line);
                    if (line.find("-") != std::string::npos)
                        break;

                    int transitNode = std::stoi(line);
                    transitNodesPerCell[x][y].push_back(transitNode);
                }
            }
        }

        for (int i = 0; i < n_nodes; i++) {
            while(true) {
                std::string line;
                std::getline(file, line);
                if (line.find("-") != std::string::npos)
                    break;


                int tnDistances = std::stoi(line);
                distancesToLocalTransitNodes[i].push_back(tnDistances);
            }   
        }
        TransitNodesData tnData = TransitNodesData(transitNodes, distancesBetweenTransitNodes, transitNodesPerCell, distancesToLocalTransitNodes, gridsize, gridsize);
        return tnData;
    }
    throw std::runtime_error("Could not open the specified file.");
}