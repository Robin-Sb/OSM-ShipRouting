#include "PBFReader.h"

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
            coastline.push_back(coast);
        }
    }

void CoastHandler::node(const osmium::Node& node) {};