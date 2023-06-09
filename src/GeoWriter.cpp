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
    "   \"properties\": {},\n"
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
        if (nodes[sources[i]].lon < -175 && nodes[targets[i]].lon > 175) 
            tLonSrc = 180 + (180 + nodes[sources[i]].lon);
        if (nodes[sources[i]].lon > 175 && nodes[targets[i]].lon < -175)
            tLonTrg = 180 + (180 + nodes[targets[i]].lon);

        if (nodes[sources[i]].lat < 50 || nodes[sources[i]].lat > 60)
            continue;
        if (tLonSrc < -15 || tLonSrc > 3) 
            continue; 
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

void GeoWriter::writeToDisk(std::string j_string, std::string &file_name) {
    std::ofstream json_stream;
    json_stream.open (file_name);
    json_stream << j_string;
    json_stream.close();
}