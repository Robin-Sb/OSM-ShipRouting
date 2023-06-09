#pragma once
#include <vector>
#include <osmium/io/pbf_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <iostream>
#include <osmium/handler.hpp>
#include <unordered_map>
#include "GraphUtils.h"

class CoastHandler : public osmium::handler::Handler {
public:
    std::vector<SingleCoast> coastlines; 
    void way(const osmium::Way& way);
    void node(const osmium::Node& node);
};

class CoastlineStitcher {
    public:
        CoastlineStitcher(std::vector<SingleCoast> &_coastlines);
        std::vector<SingleCoast> coastlines;
        std::vector<SingleCoast> updatedCoastlines;
        std::unordered_map<int, int> processedCoastlines;
        std::vector<bool> isCLActive;
        std::vector<SingleCoast> stitchCoastlines();
        std::vector<SingleCoast> stitchCL2();
    private:
        void processSingleCoastline(int i);
        void handleCaseUnseen(int i);
        int handleCaseStartMatchesEnd(int i);
        void handleCaseFinishedCoastline(int i, int coastIndexStart, int coastIndexEnd);
        void handleCaseSandwich(int i, int coastIndexStart, int coastIndexEnd);
        void handleCaseOnlyEndMatchesStart(int i, int coastIndex);
};