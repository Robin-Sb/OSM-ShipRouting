#pragma once
#include <vector>
#include <osmium/io/pbf_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <iostream>
#include <osmium/handler.hpp>
#include <unordered_map>
#include "Graph.h"


class SingleCoast {
    public:
        std::vector<Node> path;
};


class CoastHandler : public osmium::handler::Handler {
public:
    std::vector<SingleCoast> coastline; 
    void way(const osmium::Way& way);
    void node(const osmium::Node& node);
};