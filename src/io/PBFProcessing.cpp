#include "PBFProcessing.h"

void CoastHandler::way(const osmium::Way& way) {
        if (way.tags().has_tag("natural", "coastline")) {
            SingleCoast coast;
            for (const osmium::NodeRef& n : way.nodes()) {
                try {
                    Node node = Node(n.lat(), n.lon(), n.ref());
                    coast.path.push_back(node);
                } catch (osmium::invalid_location) {
                    
                }
            }
            coastlines.push_back(coast);
        }
    }

void CoastHandler::node(const osmium::Node& node) {};

CoastlineStitcher::CoastlineStitcher(std::vector<SingleCoast> &_coastlines) {
    coastlines = _coastlines;
}

std::vector<SingleCoast> CoastlineStitcher::stitchCoastlines() {
    // first páss, fill hashmap so that we can find references easily
    std::unordered_map<Node, int, Node::HashFunction> map;
    std::unordered_map<int, int> idMap;
    for (int i = 0; i < coastlines.size(); i++) {
        idMap.insert(std::make_pair(coastlines[i].path[0].id, i));
        map.insert(std::make_pair(coastlines[i].path[0], i));
    }

    std::vector<bool> isProcessed(coastlines.size(), false);
    std::vector<SingleCoast> result;
    for (int i = 0; i < coastlines.size(); i++) {
        if (isProcessed[i])
            continue;
        SingleCoast coastline = coastlines[i];
        while (true) {
            // first, try to find by lat/lng
            int nextStart = map.find(coastline.path[coastline.path.size()- 1])->second;

            if (isProcessed[nextStart]) {
                // case where there are multiple points at the same spot (degenerate) -> use id as backup
                int nextId = idMap.find(coastline.path[coastline.path.size() - 1].id)->second;
                if (nextId != i)
                    nextStart = nextId;
                else 
                    break;
                
                // case where the next point is old, but different from the last point we have seen -> avoid loop by breaking
                if (coastlines[nextStart].path[0].lat != coastline.path[coastline.path.size() - 1].lat &&
                    coastlines[nextStart].path[0].lon != coastline.path[coastline.path.size() - 1].lon)
                    break;
            }

            // only process every coastline once 
            isProcessed[nextStart] = true;
            // beginning matches end -> coastline complete
            if (nextStart == i) 
                break;
            
            // if the next point is valid, attach the coastline
            for (int j = 1; j < coastlines[nextStart].path.size(); j++) {
                coastline.path.push_back(coastlines[nextStart].path[j]);
            }
        }
        result.push_back(coastline);
    }
    return result;
}