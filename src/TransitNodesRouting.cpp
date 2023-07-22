#include "TransitNodesRouting.h"

TransitNodesRouting::TransitNodesRouting(std::shared_ptr<Graph> _graph, int _gridsize) {
    graph = _graph; 
    gridsize = _gridsize;
}


void TransitNodesRouting::debug() {
    for (int i = 0; i < gridsize; i++) {
        for (int j = 0; j < gridsize; j++) {
            for (int k = 0; k < edgeBucketsVertical[i][j].size(); k++) {
                std::cout << "x: " << std::to_string(i) << ", y: " << std::to_string(j) << ", edge: " << std::to_string(edgeBucketsVertical[i][j][k]) << std::endl;
            }
        }
    }
}

void TransitNodesRouting::findBoundaryNodes(int xIndex, int yIndex, int localIndex, std::array<std::vector<int>, 5> &cArray, std::array<std::vector<int>, 5> &indicesOfCArray, std::vector<std::pair<bool, std::tuple<int,int, RelativePosition>>> &boundaryNodes, int &n_boundaryNodes, RelativePosition relPos) {
    findBoundaryNodesDirectional(xIndex, yIndex, localIndex, cArray, indicesOfCArray, boundaryNodes, n_boundaryNodes, edgeBucketsVertical, relPos);
    findBoundaryNodesDirectional(xIndex, yIndex, localIndex, cArray, indicesOfCArray, boundaryNodes, n_boundaryNodes, edgeBucketsHorizontal, relPos);
}

void TransitNodesRouting::findBoundaryNodesDirectional(int xIndex, int yIndex, int localIndex, std::array<std::vector<int>, 5> &cArray, std::array<std::vector<int>, 5> &indicesOfCArray, std::vector<std::pair<bool, std::tuple<int,int, RelativePosition>>> &boundaryNodes, int &n_boundaryNodes, std::vector<std::vector<std::vector<int>>> &edgeBuckets, RelativePosition relPos) {
    for (int j = 0; j < edgeBuckets[xIndex][yIndex].size(); j++) {
        // discard duplicate edges
        int startIndex = graph->sources[edgeBuckets[xIndex][yIndex][j]];
        int endIndex = graph->targets[edgeBuckets[xIndex][yIndex][j]];
        if (boundaryNodes[startIndex].first || boundaryNodes[endIndex].first)
            continue;
        int nodeIndex = startIndex;
        // always take the node with smaller x value to avoid ambiguities
        // TODO: think of a better way to do this
        if (graph->nodes[endIndex].lon < graph->nodes[startIndex].lon)
            nodeIndex = endIndex;
        
        // always taking the first node here means sometimes the edge is included and sometimes it isn't
        // I am not sure what the correct thing to do is here
        // actually shouldn't matter because in each shortest path the node edge will be included i guess
        boundaryNodes[startIndex] = std::make_pair(true, std::make_tuple(localIndex, cArray[localIndex].size(), relPos));
        cArray[localIndex].push_back(0); //graph->sources[edgeBucketsVertical[xIndex][yIndex][j]]);
        indicesOfCArray[localIndex].push_back(startIndex);
        n_boundaryNodes++;
    }
}

void TransitNodesRouting::storeDistances(int cellIndex, int vIndex, std::array<std::vector<int>, 5> &cArray, std::array<std::vector<int>, 5> &indicesOfCArray, std::vector<std::vector<std::pair<int, std::unordered_map<int, int>>>> &distancesVertical, std::array<std::unordered_map<int, int>, 2> &nodeIdxToMapIdx, int lrIndex) {

    for (int i = 0; i < indicesOfCArray.size(); i++) {
        for (int j = 0; j < indicesOfCArray[i].size(); j++) {
            int nodeIndex = indicesOfCArray[i][j];
            int mapIndex;
            if (nodeIdxToMapIdx[lrIndex].find(nodeIndex) == nodeIdxToMapIdx[lrIndex].end()) {
                mapIndex = distancesVertical[cellIndex].size();
                nodeIdxToMapIdx[lrIndex][nodeIndex] = mapIndex;
                std::unordered_map<int, int> distanceData;
                distanceData[vIndex] = cArray[i][j];
                std::pair<int, std::unordered_map<int, int>> newNodeEntry = std::make_pair(nodeIndex, distanceData);
                // std::pair<int, int> data = std::make_pair(nodeIndex, distanceData);
                // newNodeEntry->second[vIndex] = data;
                distancesVertical[cellIndex].push_back(newNodeEntry);
            }
            else {
                mapIndex = nodeIdxToMapIdx[lrIndex].find(nodeIndex)->second;
                distancesVertical[cellIndex][mapIndex].second.insert(std::make_pair(vIndex, cArray[i][j]));
            }
        }
    }
}

bool TransitNodesRouting::compareYCoordinate(std::pair<int, std::unordered_map<int, int>> &value1, std::pair<int, std::unordered_map<int, int>> &value2) {
    Vec2Sphere pos1 = graph->nodes[value1.first];
    Vec2Sphere pos2 = graph->nodes[value2.first];
    return pos1.lat > pos2.lat;
}

// TODO: wraparound antimeridian and pole case
void TransitNodesRouting::findTransitNodes() {
    std::array<int, 2> cellsLeft{-2, -1};
    std::array<int, 2> cellsRight{2, 3};


    // the nodeindices have 1:1 correspondence to mapindex anyway, why do we need to store both
    
    // [GridIndexX][GridIndexY][NodeIndex(In Array)]{NodeIndex (global), {vIndex, vDistance}}
    std::vector<std::vector<std::pair<int, std::unordered_map<int, int>>>> distancesVertical(gridsize);
    // std::vector<std::vector<std::vector<std::pair<int, int>>>> distancesVertical(gridsize); 

    // first part, find distances from boundary nodes to all potential nodes
    for (int sweepIndexX = 0; sweepIndexX < gridsize; sweepIndexX++) {
        // maps the absolute node index to the node index in the distancesVertical array
        // sadly we need two maps since edges can span over multiple cells (near the poles)
        std::array<std::unordered_map<int, int>, 2> nodeIdxToMapIdx;
        // [x_index][y_index][] 
        //std::vector<std::vector<std::vector<int>>> distances (gridsize);
        for (int sweepIndexY = 0; sweepIndexY < gridsize; sweepIndexY++) {
            //std::vector<int> potentialTransitNodes;
            for (int edgeIndex = 0; edgeIndex < edgeBucketsVertical[sweepIndexX][sweepIndexY].size(); edgeIndex++) {
                // just always use the source idk know actually 
                int vIndex = graph->sources[edgeBucketsVertical[sweepIndexX][sweepIndexY][edgeIndex]];
                Vec2 v = Vec2(graph->nodes[vIndex]);
                // push all the v into one array and then sort in the end by y-coordinate 
                //potentialTransitNodes.push_back(vIndex);
                int gridCellX = std::floor(v.x * gridsize);
                int gridCellY = std::floor(v.y * gridsize);
                
                // [NodeIndex](one entry for every node){whether node is at boundary, {first dim in cArray (local index of cell), second dim in cArray (index of node), whether lower or upper}}
                std::vector<std::pair<bool, std::tuple<int,int, RelativePosition>>> boundaryNodes (graph->nodes.size(), std::make_pair(false, std::make_tuple(-1,-1, RelativePosition::UNDEF)));
                std::array<std::vector<int>, 5> cLeft;
                std::array<std::vector<int>, 5> indicesOfCLeft;
                std::array<std::vector<int>, 5> cRight;
                std::array<std::vector<int>, 5> indicesOfCRight;

                // boundary condition incase we are the edge of the grid 
                int minY = std::max(0, 2 - gridCellY) - 2;
                int maxY = std::min((gridsize - 1) - gridCellY, 2);
                
                int n_boundaryNodes = 0;
                // this only processes the vertical nodes so far -> expand to horizontal
                for (int i = minY; i <= maxY; i++) {
                    int localIdx = 4 - (i + 2);
                    for (int cell = 0; cell < 2; cell++) {
                        // special case caught due to minY and maxY
                        int yIndex = gridCellY + i;
                        // wraparound incase x becomes negative
                        int xIndexL = ((sweepIndexX + (cell - 3)) + gridsize) % gridsize;
                        int xIndexR = (sweepIndexX + cell + 2) % gridsize; 
                        findBoundaryNodes(xIndexL, yIndex, localIdx, cLeft, indicesOfCLeft, boundaryNodes, n_boundaryNodes, RelativePosition::LEFTLOWER);
                        findBoundaryNodes(xIndexR, yIndex, localIdx, cRight, indicesOfCRight, boundaryNodes, n_boundaryNodes, RelativePosition::RIGHTUPPER);
                    }
                }
                dijkstra(vIndex, boundaryNodes, cLeft, cRight, n_boundaryNodes);
                int leftCellIndex = (sweepIndexX - 2 + gridsize) % gridsize;
                storeDistances(leftCellIndex, vIndex, cLeft, indicesOfCLeft, distancesVertical, nodeIdxToMapIdx, 0);
                int rightCellIndex = (sweepIndexX + 2) % gridsize;
                storeDistances(rightCellIndex, vIndex, cRight, indicesOfCRight, distancesVertical, nodeIdxToMapIdx, 1);
            }
        }
    }
    for (int i = 0; i < distancesVertical.size(); i++) {
        std::sort(distancesVertical[i].begin(), distancesVertical[i].end(), [this](std::pair<int, std::unordered_map<int, int>> &value1, std::pair<int, std::unordered_map<int, int>> &value2) {return compareYCoordinate(value1, value2); });
    }

    std::set<int> transitNodesPre;

    for (int i = 0; i < distancesVertical.size(); i++) {
        int minYIndex = 0;
        int xComp = (i + 4) % gridsize;
        for (int j = 0; j < distancesVertical[i].size(); j++) {
            // TODO: make separate project y and x functions in utils and call them here
            int yGridCell = std::floor(Vec2(graph->nodes[distancesVertical[i][j].first]).y * gridsize);
            int maxCell = yGridCell - 2;
            for (int k = minYIndex; k < distancesVertical[xComp].size(); k++) { 
                int yGridCellRef = std::floor(Vec2(graph->nodes[distancesVertical[xComp][k].first]).y * gridsize);
                if (yGridCellRef > yGridCell + 2) {
                    minYIndex++;
                    continue;
                }
                if (yGridCellRef < maxCell)
                    break;
                int minDist = 2000000000;
                int minVIndex = -1;
                for (auto const &v : distancesVertical[i][j].second) {
                    // don't take max int value here because of overflow
                    int ref_dist = 2000000000;
                    if (distancesVertical[xComp][k].second.find(v.first) != distancesVertical[xComp][k].second.end())
                        ref_dist = distancesVertical[xComp][k].second.at(v.first);
                    int orig_dist = v.second;
                    if (ref_dist + orig_dist < minDist) {
                        minDist = ref_dist + orig_dist;
                        minVIndex = v.first;
                    }
                }
                transitNodesPre.insert(minVIndex);
                //transitNodes.push_back(minVIndex);
            }
        }
    }

    for (auto& tn : transitNodesPre) {
        if (tn != -1)
            transitNodes.push_back(tn);
    }
}

// modified dijkstra which stops after all boundaryNodes have been explored
void TransitNodesRouting::dijkstra(int startIndex, std::vector<std::pair<bool, std::tuple<int,int, RelativePosition>>> &boundaryNodes, std::array<std::vector<int>, 5> &cLeft, std::array<std::vector<int>, 5> &cRight, int n_boundaryNodes) {
    std::vector<int> prev;
    std::vector<int> dist;
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;
    for (int i = 0; i < graph->nodes.size(); i++) {
        int max_int = 2147483647;
        dist.push_back(max_int);
        prev.push_back(-1);
    }
    dist[startIndex] = 0;
    pq.push(std::make_pair(0, startIndex));
    std::vector<bool> explored (graph->nodes.size(), false);
    int counter = 0;

    while (!pq.empty()) {
        std::pair<int, int> node = pq.top();
        pq.pop();
        int u = node.second;
        if (explored[u])
            continue;
        if (counter == n_boundaryNodes) 
            break;
        if (boundaryNodes[u].first) {
            counter++;
            if (std::get<2>(boundaryNodes[u].second) == RelativePosition::LEFTLOWER)
                cLeft[std::get<0>(boundaryNodes[u].second)][std::get<1>(boundaryNodes[u].second)] = dist[u];
            else if (std::get<2>(boundaryNodes[u].second) == RelativePosition::RIGHTUPPER)
                cRight[std::get<0>(boundaryNodes[u].second)][std::get<1>(boundaryNodes[u].second)] = dist[u]; 
            boundaryNodes[u].first = false;
        }
        explored[u] = true;
        for (int i = graph->offsets[u]; i < graph->offsets[u + 1]; i++) {
            int v = graph->targets[i];
            if (explored[v])
                continue;

            int altDist = dist[u] + graph->costs[v];
            if (altDist < dist[v]) {
                dist[v] = altDist;
                prev[v] = u;
                pq.push(std::make_pair(dist[v], v));
            }
        }
    }
}

// find the smaller and bigger x value
// then check if smaller and bigger x value are in the same cell
// if they are not, put the edge towards the bucket with the bigger x value (both by x and y direction)
void TransitNodesRouting::fillBucketsVertical(Vec2 start, Vec2 end, int edgeIndex) { 
    float smallerX;
    float biggerX;
    int yIndex;
    if (start.x > end.x) {
        smallerX = end.x;
        biggerX = start.x;
        yIndex = std::floor(start.y * gridsize);
    } else {
        smallerX = start.x;
        biggerX = end.x;
        yIndex = std::floor(end.y * gridsize);
    }
    int firstIndexX = std::ceil(smallerX * gridsize);
    int lastIndexX = std::ceil(biggerX * gridsize);
    for (int counter = 0; firstIndexX + counter < lastIndexX; counter++) {
        edgeBucketsVertical[firstIndexX + counter][yIndex].push_back(edgeIndex);
    }
}

// analog to vertical with y and x swapped
void TransitNodesRouting::fillBucketsHorizontal(Vec2 start, Vec2 end, int edgeIndex) {
    float smallerY;
    float biggerY;
    float xIndex;
    if (start.y > end.y) {
        smallerY = end.y;
        biggerY = start.y;
        xIndex = std::floor(start.x * gridsize);
    } else {
        smallerY = start.y;
        biggerY = end.y;
        xIndex = std::floor(end.x * gridsize);
    }
    int firstIndexY = std::ceil(smallerY * gridsize);
    int lastIndexY = std::ceil(biggerY * gridsize);
    for (int counter = 0; firstIndexY + counter < lastIndexY; counter++) {
        edgeBucketsHorizontal[xIndex][firstIndexY + counter].push_back(edgeIndex);
    }
}

std::vector<Vec2Sphere> TransitNodesRouting::transformBack() {
    std::vector<Vec2Sphere> nodes;
    for (int x = 0; x < gridsize; x++) {
        for (int y = 0; y < gridsize; y++) {
            for (int i = 0; i < edgeBucketsVertical[x][y].size(); i++) {
                nodes.push_back(graph->nodes[graph->sources[edgeBucketsVertical[x][y][i]]]);
            }
        }
    }
    return nodes;
}

// TODO: I think antimeridian case does not work yet -> Fix later
void TransitNodesRouting::findEdgeBuckets() {
    edgeBucketsHorizontal = std::vector<std::vector<std::vector<int>>> (gridsize, std::vector<std::vector<int>> (gridsize));
    edgeBucketsVertical = std::vector<std::vector<std::vector<int>>> (gridsize, std::vector<std::vector<int>> (gridsize));
    for (int i = 0; i < graph->sources.size(); i++) {
        // constructor of Vec2 creates an equidistant cylindrical projection on a unit interval [0, 1]
        Vec2 startProj = Vec2(graph->nodes[graph->sources[i]]);
        Vec2 endProj = Vec2(graph->nodes[graph->targets[i]]);
        fillBucketsVertical(startProj, endProj, i);
        fillBucketsHorizontal(startProj, endProj, i);
    } 
}