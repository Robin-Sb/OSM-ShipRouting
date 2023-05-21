#include "GeoWriter.h"

std::string GeoWriter::buildPolygonGeoJson(std::vector<SingleCoast> coastlines) {
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

std::string GeoWriter::buildNodesGeoJson(std::vector<Vec2Sphere> nodes) {
    std::string out = "{ \"type\": \"FeatureCollection\"," 
    "\"features\": [{"
    "\"type\": \"Feature\","
    "\"properties\":{\"marker-color\": \"#006600\"},"
    "\"geometry\":{"
    "\"type\": \"MultiPoint\","
    "\"coordinates\": [";
    for (int i = 0; i < nodes.size(); i++) {
        out += "[" + std::to_string(nodes[i].lon) + ",\n" + std::to_string(nodes[i].lat) + "\n]";

        if (i != nodes.size() - 1)
            out+=",";
        out += "\n";
    }
    out += "]}}]}";
    return out;
}

void GeoWriter::writeToDisk(std::string j_string, std::string file_name) {
    std::ofstream json_stream;
    json_stream.open (file_name);
    json_stream << j_string;
    json_stream.close();

}