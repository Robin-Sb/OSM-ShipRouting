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
            coastlines.push_back(coast);
        }
    }

void CoastHandler::node(const osmium::Node& node) {};

CoastlineStitcher::CoastlineStitcher(std::vector<SingleCoast> _coastlines) {
    coastlines = _coastlines;
}

std::vector<SingleCoast> CoastlineStitcher::stitchCoastlines() {
    // std::unordered_map<int, int> processedCoastlines;
    // std::vector<SingleCoast> updatedCoastlines;
    // std::vector<bool> isCLActive;

    for (int i = 0; i < coastlines.size(); i++) {
        processSingleCoastline(i);
    }
    // filter inactive coastlines
    std::vector<SingleCoast> result;
    for (int i = 0; i < updatedCoastlines.size(); i++) {
        if (isCLActive[i])
            result.push_back(updatedCoastlines[i]);
    }
    return result;
}

void CoastlineStitcher::handleCaseUnseen(int i) {
    // append the coastline
    updatedCoastlines.push_back(coastlines[i]);
    // manage inactive coastlines for the sandwich case
    isCLActive.push_back(false);
    // store first and list node id in hashmap
    processedCoastlines.insert(std::make_pair(coastlines[i].path[0].id, updatedCoastlines.size() - 1));
    processedCoastlines.insert(std::make_pair(coastlines[i].path[coastlines[i].path.size() - 1].id, updatedCoastlines.size() - 1));
}

// the first node of the currently checked coastline is already in the hashmap
// this must be the end of another coastline -> just append
int CoastlineStitcher::handleCaseStartMatchesEnd(int i) {
    int coastIndex = processedCoastlines.find(coastlines[i].path[0].id)->second;
    // just append all nodes to the new coastline
    for (int j = 1; j < coastlines[i].path.size(); j++) {
        updatedCoastlines[coastIndex].path.push_back(coastlines[i].path[j]);
    }
    // erase the last vertex from the old coastline
    processedCoastlines.erase(coastlines[i].path[0].id);
    // the new endpoint of the coastline is the last vertex of the new coastline
    processedCoastlines.insert(std::make_pair(coastlines[i].path[coastlines[i].path.size() - 1].id, coastIndex));
    return coastIndex;
}

void CoastlineStitcher::handleCaseFinishedCoastline(int i, int coastIndexStart, int coastIndexEnd) {
    // it kind of does not matter what is erased here, none of these will be called later and they map to the same path anyways
    processedCoastlines.erase(coastlines[i].path[0].id);
    processedCoastlines.erase(updatedCoastlines[coastIndexEnd].path[0].id);
    // only finished coastlines are set to active
    isCLActive[coastIndexStart] = true;
}

void CoastlineStitcher::handleCaseSandwich(int i, int coastIndexStart, int coastIndexEnd) {
    std::vector<Node> newPath;
    for (int j = 0; j < updatedCoastlines[coastIndexEnd].path.size(); j++) {
        newPath.push_back(updatedCoastlines[coastIndexEnd].path[j]);
    }

    for (int j = 1; j < updatedCoastlines[coastIndexStart].path.size(); j++) {
        newPath.push_back(updatedCoastlines[coastIndexStart].path[j]);
    }

    updatedCoastlines[coastIndexStart].path = newPath;
    // since the coastline belonging to coastIndexEnd is inactive now, reset its value to coastIndexStart
    processedCoastlines.erase(coastlines[i].path[0].id);
    processedCoastlines.erase(coastlines[i].path[coastlines[i].path.size() - 1].id);
    processedCoastlines.erase(updatedCoastlines[coastIndexEnd].path[0].id);
    processedCoastlines.insert(std::make_pair(updatedCoastlines[coastIndexEnd].path[0].id, coastIndexStart));
}

void CoastlineStitcher::handleCaseOnlyEndMatchesStart(int i, int coastIndex) {
    std::vector<Node> newPath;
    for (int j = 0; j < coastlines[i].path.size(); j++) {
        newPath.push_back(coastlines[i].path[j]);
    }

    for (int j = 1; j < updatedCoastlines[coastIndex].path.size(); j++) {
        newPath.push_back(updatedCoastlines[coastIndex].path[j]);
    }
    processedCoastlines.erase(updatedCoastlines[coastIndex].path[0].id);
    updatedCoastlines[coastIndex].path = newPath;
    processedCoastlines.insert(std::make_pair(coastlines[i].path[0].id, coastIndex));
}

void CoastlineStitcher::processSingleCoastline(int i) {
    int pathLength = coastlines[i].path.size();
    bool firstNodeExists = !(processedCoastlines.find(coastlines[i].path[0].id) == processedCoastlines.end());
    bool lastNodeExists = !(processedCoastlines.find(coastlines[i].path[pathLength - 1].id) == processedCoastlines.end());
    // standard case -> new coastline, append to the existing coastline and store the index in hashmap
    if (!firstNodeExists && !lastNodeExists) {
        handleCaseUnseen(i);
    }
    int coastIndexEnd;
    if (firstNodeExists) {
        coastIndexEnd = handleCaseStartMatchesEnd(i);
    }

    if (lastNodeExists) {
        int coastIndexStart = processedCoastlines.find(coastlines[i].path[coastlines[i].path.size() - 1].id)->second;
        if (firstNodeExists) { 
            // start and end are the same -> finished coastline -> do nothing
            if (coastIndexStart == coastIndexEnd) {
                handleCaseFinishedCoastline(i, coastIndexStart, coastIndexEnd);
                return;
            }
            // start and end are not the same -> sandwich case
            handleCaseSandwich(i, coastIndexStart, coastIndexEnd);
        } else {
            handleCaseOnlyEndMatchesStart(i, coastIndexStart);
        }
    }

}