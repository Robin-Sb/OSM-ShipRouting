#include "Utils.h"

Vec2Sphere::Vec2Sphere(float _lat, float _lon) {
    lat = _lat;
    lon = _lon;
}

float Vec2Sphere::dist(Vec2Sphere ref, Vec2Sphere comp) {
    // earth radius in m
    int radius = 6371000;
    float pi = 3.14159265359;
    float deg_to_rad = pi / 180;
    float lat1_rad = ref.lat * deg_to_rad;
    float lat2_rad = comp.lat * deg_to_rad;
    float diff_lat = (comp.lat - ref.lat) * deg_to_rad;
    float diff_lon = (comp.lon - ref.lon) * deg_to_rad;

    float a = std::sin(diff_lat/2) * std::sin(diff_lat/2) + std::cos(lat1_rad) * std::cos(lat2_rad) * std::sin(diff_lon/2) * std::sin(diff_lon/2);
    float c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    return radius * c;
}

SphericalGrid::SphericalGrid(std::shared_ptr<std::vector<Vec2Sphere>> _nodes): nodes(_nodes) {
    for (int i = 0; i < nodes->size(); i++) {
        addPoint(i, (*nodes)[i]);
    }
}

void SphericalGrid::addPoint(int nodeIdx, Vec2Sphere loc) {
    cells[getIndexLon(loc)][getIndexLat(loc)].push_back(nodeIdx);
}

std::vector<int> SphericalGrid::getPointsAt(Vec2Sphere loc) {
    return cells[getIndexLon(loc)][getIndexLat(loc)];
}

int SphericalGrid::getIndexLon(Vec2Sphere loc) {
    int n_lon = cells.size();
    return std::floor((loc.lon + 180)/(360 / (n_lon - 1)));
}

int SphericalGrid::getIndexLat(Vec2Sphere loc) {
    int n_lat = cells[0].size();
    return std::floor((loc.lat + 90)/(180 / (n_lat - 1)));
}

FoundNodes SphericalGrid::findClosestPoints(Vec2Sphere loc) {
    std::shared_ptr<std::array<std::array<std::vector<int>, 180>, 360>> _cells = std::make_shared<std::array<std::array<std::vector<int>, 180>, 360>>(cells);
    //std::shared_ptr<std::vector<Vec2Sphere>> _nodes = std::make_shared<std::vector<Vec2Sphere>>(nodes);
    CellSearch cellSearch = CellSearch(200000, loc, _cells, nodes, getIndexLat(loc), getIndexLon(loc));
    return cellSearch.startSearch();
}

// kind of not DRY, but this handles the "special" case to look at its own cell (otherwise the calculation is repeated 4 times)
FoundNodes CellSearch::startSearch() {
    FoundNodes finalResult; 
    finalResult.leftBottom = expandSearch(startIdxLat, startIdxLon, -1, -1, SearchDirection::LEFT_BOTTOM, 0).index;
    finalResult.rightBottom = expandSearch(startIdxLat, startIdxLon, -1, 1, SearchDirection::RIGHT_BOTTOM, 0).index;
    finalResult.leftTop = expandSearch(startIdxLat, startIdxLon, 1, -1, SearchDirection::LEFT_TOP, 0).index;
    finalResult.rightTop = expandSearch(startIdxLat, startIdxLon, 1, 1, SearchDirection::RIGHT_TOP, 0).index;
    return finalResult;
}

SearchResult CellSearch::updateResult(SearchResult bestResult, SearchDirection searchDir, Vec2Sphere ref, Vec2Sphere comp, int value) {
    switch(searchDir) {
        case SearchDirection::LEFT_BOTTOM: {
            if (!(checkLeft(ref, comp) && checkBottom(ref, comp)))
                return bestResult; 
            break;
        };
        case SearchDirection::LEFT_TOP: {
            if (!(checkLeft(ref, comp) && checkTop(ref, comp)))
                return bestResult;
            break;
        };
        case SearchDirection::RIGHT_BOTTOM: {
            if (!(checkRight(ref, comp) && checkBottom(ref, comp)))
                return bestResult;
            break;
        };
        case SearchDirection::RIGHT_TOP: {
            if (!(checkRight(ref, comp) && checkTop(ref, comp))) 
                return bestResult;
            break;
        }
    }
    float distToRef = Vec2Sphere::dist(comp, ref);
    if (distToRef < bestResult.dist) {
        bestResult = SearchResult(value, distToRef);
        //bestResult = SearchResult(cells[idxLon][idxLat][i], distToRef);
    }
    return bestResult;
}

SearchResult CellSearch::expandSearch(int idxLat, int idxLon, int dirLat, int dirLon, SearchDirection searchDir, int iterCount) {
    // we are at the edge of the map -> there won't be any nodes there
    if (idxLat < 0 || idxLat > (*cells)[0].size()) 
        return SearchResult(-1, range + 1);
    
    // one full wraparound -> back at starting cell without finding any vertices
    if ((dirLat != 0 && dirLon == 0 && iterCount >= 180) || (dirLat == 0 && dirLon != 0 && iterCount >= 360))
        return SearchResult(-1, range + 1);

    SearchResult bestResult = SearchResult(-1, range + 1);
    for (int i = 0; i < (*cells)[idxLon][idxLat].size(); i++) {
        Vec2Sphere pos = (*nodes)[(*cells)[idxLon][idxLat][i]];
        bestResult = updateResult(bestResult, searchDir, loc, pos, (*cells)[idxLon][idxLat][i]);
    }
    int dimLon = (*cells).size();
    int dimLat = (*cells)[0].size();
    // wraparound at the end of the grid
    int newIdxLon = (idxLon + dirLon + dimLon) % dimLon; 
    //int newIdxLat = (idxLat + dirLat + dimLat) % dimLat;
    // fix negative sign lat/long
    float distToBorderDiagonal = Vec2Sphere::dist(loc, getBoundaryCoords(idxLat, idxLon, dirLat, dirLon));
    SearchResult stdBest = SearchResult(-1, range + 1);
    // standard case -> without expansion
    if (distToBorderDiagonal < range) {
        SearchResult diagBest = expandSearch(idxLat + dirLat, newIdxLon, dirLat, dirLon, searchDir, iterCount + 1);
        if (diagBest.dist < bestResult.dist)
            bestResult = diagBest;
    }

    // multidirectional case -> expand
    SearchResult sideBest = SearchResult(-1, range + 1);
    if (dirLon != 0 && dirLat != 0) {
        float distToBorderLonDir = Vec2Sphere::dist(loc, getBoundaryCoords(idxLat, idxLon, 0, dirLon));
        if (distToBorderLonDir < range) {
            SearchResult expLon = expandSearch(idxLat, newIdxLon, 0, dirLon, searchDir, 0);
            if (expLon.dist < bestResult.dist)
                bestResult = expLon;
        }
        
        SearchResult expLat = SearchResult(-1, range + 1);
        float distToBorderLatDir = Vec2Sphere::dist(loc, getBoundaryCoords(idxLat, idxLon, dirLat, 0));
        if (distToBorderLatDir < range) {
            SearchResult expLat = expandSearch(idxLat + dirLat, idxLon, dirLat, 0, searchDir, 0);
            if (expLat.dist < bestResult.dist)
                bestResult = expLat;
        }
    } 
    return bestResult;
}

Vec2Sphere CellSearch::getBoundaryCoords(int idxLat, int idxLon, int dirLat, int dirLon) {
    Vec2Sphere boundaryLoc = Vec2Sphere(loc.lat, loc.lon);
    int dimLon = (*cells).size();
    int dimLat = (*cells)[0].size();
    float dx = 360 / dimLon;
    float dy = 180 / dimLat;

    if (dirLat == -1) 
        boundaryLoc.lat = (idxLat - 90) * dy;
    else if (dirLat == 1) 
        boundaryLoc.lat = ((idxLat + 1 + dimLat) % dimLat - 90) * dy;
    if (dirLon == -1) 
        boundaryLoc.lon = (idxLon - 180) * dx;
    else if (dirLon == 1)
        boundaryLoc.lon = ((idxLon + 1 + dimLon) % dimLon - 180) * dy;
    return boundaryLoc;
}

bool CellSearch::checkLeft(Vec2Sphere ref, Vec2Sphere comp) {
    // special case at the border
    if (ref.lon > 0 && comp.lon < 0)
        return true;
    else if (comp.lon < ref.lon)
        return true;
    return false;
}

bool CellSearch::checkRight(Vec2Sphere ref, Vec2Sphere comp) {
    if (ref.lon < 0 && comp.lon > 0)
        return true;
    else if (comp.lon > ref.lon)
        return true;
    return false;
}

bool CellSearch::checkBottom(Vec2Sphere ref, Vec2Sphere comp) {
    if (comp.lat < ref.lat)
        return true;
    return false;
}

bool CellSearch::checkTop(Vec2Sphere  ref, Vec2Sphere comp) {
    if (comp.lat > ref.lat) 
        return true;
    return false;
}