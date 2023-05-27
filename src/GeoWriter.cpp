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

std::string GeoWriter::buildGraphGeoJson(std::vector<Vec2Sphere> &nodes, std::vector<int> &sources, std::vector<int> &targets, std::vector<Vec2Sphere> &drawNodes) {
    std::string out = "{\"type\": \"FeatureCollection\",\n"
    "\"features\":[ \n";
    for (int i = 0; i < sources.size(); i++) {
        out += "{";
        out += "    \"type\": \"Feature\",\n"
        "    \"geometry\": {\n"
        "       \"type\": \"LineString\",\n"
        "       \"coordinates\": [\n";
        float tLonSrc = nodes[sources[i]].lon;
        float tLonTrg = nodes[targets[i]].lon;
        if (nodes[sources[i]].lon < -175 && nodes[targets[i]].lon > 175) 
            tLonSrc = 180 + (180 + nodes[sources[i]].lon);
        if (nodes[sources[i]].lon > 175 && nodes[targets[i]].lon < -175)
            tLonTrg = 180 + (180 + nodes[targets[i]].lon);
        out += "[" + std::to_string(tLonSrc) + "," + std::to_string(nodes[sources[i]].lat) + "],\n";
        out += "[" + std::to_string(tLonTrg) + "," + std::to_string(nodes[targets[i]].lat) + "]\n";
        out += "]}, \"properties\": {}}";
        if (i < sources.size() - 1) {
            out += ",";
        }
    }
    bool drawPoints = false;
    if (drawPoints) {
        out += ",";
        for (int i = 0; i < drawNodes.size(); i++) {
            out += "{";
            out += "    \"type\": \"Feature\",\n"
            "    \"geometry\": {\n"
            "       \"type\": \"Point\",\n"
            "       \"coordinates\": \n";
            out += "[" + std::to_string(drawNodes[i].lon) + "," + std::to_string(drawNodes[i].lat) + "]\n";
            out += "}, \"properties\": {}}";
            if (i < drawNodes.size() - 1) {
                out += ",";
            }
        }

    }

    out += "]}";
    return out;
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

void GeoWriter::writeToDisk(std::string j_string, std::string file_name) {
    std::ofstream json_stream;
    json_stream.open (file_name);
    json_stream << j_string;
    json_stream.close();
}