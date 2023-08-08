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

std::string TransitNodesRouting::getTnList() {
    std::string out;
    for (int i = 0; i < localTransitNodes.size(); i++) {
        out += std::to_string(graph->nodes[i].lat) + ", " + std::to_string(graph->nodes[i].lon) + "\n";
        for (int j = 0; j < localTransitNodes[i].size(); j++) {
            int tnNode = localTransitNodes[i][j].nodeIndex;
            out += "\t" + std::to_string(graph->nodes[tnNode].lat) + ", " + std::to_string(graph->nodes[tnNode].lon) + ", dist: " + std::to_string(localTransitNodes[i][j].distanceToV) + "\n"; 
        }
    }
    return out;
}

void TransitNodesRouting::findBoundaryNodesDirectional(int xIndex, int yIndex, std::vector<NodeDistance> &cArray, std::vector<BoundaryNodeData> &boundaryNodes, int &n_boundaryNodes, std::vector<std::vector<std::vector<int>>> &edgeBuckets, RelativePosition relPos) {
    for (int j = 0; j < edgeBuckets[xIndex][yIndex].size(); j++) {
        // discard duplicate edges
        int startIndex = graph->sources[edgeBuckets[xIndex][yIndex][j]];
        int endIndex = graph->targets[edgeBuckets[xIndex][yIndex][j]];
        // the edge in the opposite direction was found -> skip this one
        if (boundaryNodes[startIndex].isAtCellBoundary || boundaryNodes[endIndex].isAtCellBoundary)
            continue;
        int nodeIndex = startIndex;
        // always take the node with smaller x value to avoid ambiguities
        // TODO: think of a better way to do this
        if (graph->nodes[endIndex].lon < graph->nodes[startIndex].lon)
            nodeIndex = endIndex;
        
        // always taking the first node here means sometimes the edge is included and sometimes it isn't
        // I am not sure what the correct thing to do is here
        // actually shouldn't matter because in each shortest path the node edge will be included i guess
        boundaryNodes[nodeIndex] = BoundaryNodeData(true, cArray.size(), relPos);
        cArray.push_back(NodeDistance(nodeIndex, 0));
        //indicesOfCArray.push_back(nodeIndex);
        n_boundaryNodes++;
    }
}

// here we should likely go for a different paradigm (store all distances)
void TransitNodesRouting::storeDistances(int cellIndex, int vIndex, std::vector<NodeDistance> &cArray, std::vector<DistanceData> &distances, std::unordered_map<int, int> &nodeToIdxMap) {
    for (int j = 0; j < cArray.size(); j++) {
        int nodeIndex = cArray[j].nodeIndex;
        int mapIndex;
        if (nodeToIdxMap.find(nodeIndex) == nodeToIdxMap.end()) {
            mapIndex = distances.size();
            nodeToIdxMap[nodeIndex] = mapIndex;
            std::unordered_map<int, int> distanceData;
            distanceData[vIndex] = cArray[j].distanceToV;
            DistanceData newNodeEntry = DistanceData(nodeIndex, distanceData);
            distances.push_back(newNodeEntry);
        }
        else {
            mapIndex = nodeToIdxMap.find(nodeIndex)->second;
            distances[mapIndex].distanceToV.insert(std::make_pair(vIndex, cArray[j].distanceToV));
        }
    }
}

bool TransitNodesRouting::compareCoordinates(DistanceData &value1, DistanceData &value2, bool sortByY) {
    Vec2Sphere pos1 = graph->nodes[value1.referenceNodeIndex];
    Vec2Sphere pos2 = graph->nodes[value2.referenceNodeIndex];
    if (sortByY)
        return pos1.lat > pos2.lat;
    else
        return pos1.lon > pos2.lon;
}

// void TransitNodesRouting::findBoundaryNodes() {
//     // special case caught due to minY and maxY
//     int yIndex = sweepIndexY + i;
//     // wraparound incase x becomes negative
//     int xIndexL = (sweepIndexX + cell - 3 + gridsize) % gridsize;
//     int xIndexR = (sweepIndexX + cell + 2) % gridsize; 
//     findBoundaryNodesDirectional(xIndexL, yIndex, cLeft, boundaryNodes, n_boundaryNodes, edgeBucketsVertical, RelativePosition::LEFTLOWER);
//     findBoundaryNodesDirectional(xIndexR, yIndex, cRight, boundaryNodes, n_boundaryNodes, edgeBucketsVertical, RelativePosition::RIGHTUPPER);

// }

std::vector<NodeDistance> TransitNodesRouting::processSingleNodeVertical(int sweepIndexX, int sweepIndexY, int vIndex, std::vector<DistanceData> &distancesLeft, std::vector<DistanceData> &distancesRight, std::array<std::unordered_map<int, int>, 2>& nodeIdxToMapIdx)  {    
    std::vector<BoundaryNodeData> boundaryNodes (graph->nodes.size());

    // cLeft and cRight are flat arrays with all distances to the nodes at the cell boundaries 
    std::vector<NodeDistance> cLeft;
    std::vector<NodeDistance> cRight;

    // boundary condition incase we are the edge of the grid 
    int minY = std::max(0, 2 - sweepIndexY) - 2;
    int maxY = std::min((gridsize - 1) - sweepIndexY, 2);
    
    int n_boundaryNodes = 0;
    // this only processes the vertical nodes so far -> expand to horizontal
    for (int i = minY; i <= maxY; i++) {
        int localIdx = 4 - (i + 2);
        for (int cell = 0; cell < 2; cell++) {
            // special case caught due to minY and maxY
            int yIndex = sweepIndexY + i;
            // wraparound incase x becomes negative
            int xIndexL = (sweepIndexX + cell - 3 + gridsize) % gridsize;
            int xIndexR = (sweepIndexX + cell + 2) % gridsize; 
            findBoundaryNodesDirectional(xIndexL, yIndex, cLeft, boundaryNodes, n_boundaryNodes, edgeBucketsVertical, RelativePosition::LEFTLOWER);
            findBoundaryNodesDirectional(xIndexR, yIndex, cRight, boundaryNodes, n_boundaryNodes, edgeBucketsVertical, RelativePosition::RIGHTUPPER);
        }
    }

    // horizontally, we need to look at 6 (six) cell boundaries
    for (int i = minY; i <= std::min(maxY + 1, (gridsize - 1) - sweepIndexY); i++) { 
        int yIndex = sweepIndexY + i;

        int xIndexL = (sweepIndexX - 3 + gridsize) % gridsize;
        int xIndexR = (sweepIndexX + 2) % gridsize;
        findBoundaryNodesDirectional(xIndexL, yIndex, cLeft, boundaryNodes, n_boundaryNodes, edgeBucketsHorizontal, RelativePosition::LEFTLOWER);
        findBoundaryNodesDirectional(xIndexR, yIndex, cRight, boundaryNodes, n_boundaryNodes, edgeBucketsHorizontal, RelativePosition::RIGHTUPPER);
    }
    std::vector<NodeDistance> dijkstraResults = dijkstra(vIndex, boundaryNodes, cLeft, cRight, n_boundaryNodes);
    storeDistances(sweepIndexX, vIndex, cLeft, distancesLeft, nodeIdxToMapIdx[0]);
    storeDistances(sweepIndexX, vIndex, cRight, distancesRight, nodeIdxToMapIdx[1]);
    return dijkstraResults;
}


// // TODO: refactor vertical in such a way that this function can be removed
// void TransitNodesRouting::processSingleNodeHorizontal(int sweepIndexX, int sweepIndexY, int edgeIndex, std::vector<std::array<std::vector<std::pair<int, std::unordered_map<int, int>>>, 2>> &distancesHorizontal, std::array<std::unordered_map<int, int>, 2>& nodeIdxToMapIdx) {
//     // just always use the source idk know actually 
//     int vIndex = graph->sources[edgeBucketsHorizontal[sweepIndexX][sweepIndexY][edgeIndex]];
    
//     // [NodeIndex](one entry for every node){whether node is at boundary, {first dim in cArray (index of node), whether lower or upper}}
//     std::vector<std::pair<bool, std::pair<int, RelativePosition>>> boundaryNodes (graph->nodes.size(), std::make_pair(false, std::make_pair(-1, RelativePosition::UNDEF)));

//     // cLeft and cRight are flat arrays with all distances to the nodes at the cell boundaries 
//     std::vector<int> cLeft;
//     std::vector<int> cRight;

//     // indicesOfCLeft and indicesOfCRight are arrays which map the indices in cLeft and cRight back to their node id
//     std::vector<int> indicesOfCLeft;
//     std::vector<int> indicesOfCRight;

//     // boundary condition incase we are the edge of the grid 
//     int minY = std::max(0, 2 - sweepIndexY) - 2;
//     int maxY = std::min((gridsize - 1) - sweepIndexY, 2);
    
//     int n_boundaryNodes = 0;
//     // this only processes the vertical nodes so far -> expand to horizontal
//     for (int i = -2; i <= 2; i++) {
//         int localIdx = 4 - (i + 2);
//         for (int cell = 0; cell < 2; cell++) {
//             // special case caught due to minY and maxY
//             int yIndexU = sweepIndexY + cell + 2;
//             int yIndexD = sweepIndexY + cell - 3;
//             // wraparound incase x becomes negative
//             int xIndex = (sweepIndexX + i + gridsize) % gridsize;
            
//             findBoundaryNodesDirectional(xIndex, yIndexD, cLeft, indicesOfCLeft, boundaryNodes, n_boundaryNodes, edgeBucketsHorizontal, RelativePosition::LEFTLOWER);
//             findBoundaryNodesDirectional(xIndex, yIndexU, cRight, indicesOfCRight, boundaryNodes, n_boundaryNodes, edgeBucketsHorizontal, RelativePosition::RIGHTUPPER);
//         }
//     }

//     // horizontally, we need to look at 6 (six) cell boundaries
//     for (int i = -2; i <= 3; i++) { 
//         int yIndexU = sweepIndexY + 3;
//         int yIndexD = sweepIndexY - 2;

//         int xIndex = (sweepIndexX + i + gridsize) % gridsize;
//         findBoundaryNodesDirectional(xIndex, yIndexD, cLeft, indicesOfCLeft, boundaryNodes, n_boundaryNodes, edgeBucketsVertical, RelativePosition::LEFTLOWER);
//         findBoundaryNodesDirectional(xIndex, yIndexU, cRight, indicesOfCRight, boundaryNodes, n_boundaryNodes, edgeBucketsVertical, RelativePosition::RIGHTUPPER);
//     }
//     dijkstra(vIndex, boundaryNodes, cLeft, cRight, n_boundaryNodes);
//     int bottomCellIndex = (sweepIndexY - 2 + gridsize) % gridsize;
//     storeDistances(sweepIndexY, vIndex, cLeft, indicesOfCLeft, distancesHorizontal, nodeIdxToMapIdx, 0);
//     int topCellIndex = (sweepIndexY + 2) % gridsize;
//     storeDistances(sweepIndexY, vIndex, cRight, indicesOfCRight, distancesHorizontal, nodeIdxToMapIdx, 1);
// }

void TransitNodesRouting::sortDescending(std::vector<DistanceData> &distances, bool sortByY) {
    std::sort(distances.begin(), distances.end(), 
    [this, sortByY](DistanceData &value1, DistanceData &value2) {
        return compareCoordinates(value1, value2, sortByY); 
    });
}

// TODO: wraparound antimeridian and pole case
// TODO: maybe the whole transit nodes search can be refactored into a new class
void TransitNodesRouting::sweepLineTransitNodes() {
    // the nodeindices have 1:1 correspondence to mapindex anyway, why do we need to store both
    // [GridIndexX][GridIndexY][NodeIndex(In Array)]{NodeIndex (global), {vIndex, vDistance}}
    // set length of local transit nodes to avoid segfault
    localTransitNodes = std::vector<std::vector<NodeDistance>> (graph->nodes.size());

    // first part, find distances from boundary nodes to all potential nodes
    for (int sweepIndexX = 0; sweepIndexX < gridsize; sweepIndexX++) {
        std::vector<DistanceData> distancesLeft;
        std::vector<DistanceData> distancesRight;

        std::vector<int> vs;
        std::unordered_map<int, std::vector<NodeDistance>> distancesToNearestTransitNode;

        //std::vector<std::vector<std::pair<int, std::unordered_map<int, int>>>> distancesVertical(gridsize);
        std::vector<std::array<std::vector<std::pair<int, std::unordered_map<int, int>>>, 2>> distancesHorizontal(gridsize);

        std::array<std::unordered_map<int, int>, 2> nodeIdxToMapIdxVertical;
        // maps the absolute node index to the node index in the distances array
        // we need two maps since edges can span over multiple cells (near the poles)
        for (int sweepIndexY = 0; sweepIndexY < gridsize; sweepIndexY++) {
            std::array<std::unordered_map<int, int>, 2> nodeIdxToMapIdxHorizontal;
            // process nodes which cross a vertical grid line
            for (int edgeIndex = 0; edgeIndex < edgeBucketsVertical[sweepIndexX][sweepIndexY].size(); edgeIndex++) {
                // just always use the source idk know actually 
                int vIndex = graph->sources[edgeBucketsVertical[sweepIndexX][sweepIndexY][edgeIndex]];
                std::vector<NodeDistance> dijkstraResults = processSingleNodeVertical(sweepIndexX, sweepIndexY, vIndex, distancesLeft, distancesRight, nodeIdxToMapIdxVertical);
                distancesToNearestTransitNode[vIndex] = dijkstraResults;
                vs.push_back(vIndex);
            }
        }
        // sortDescending(distancesLeft, true);
        // sortDescending(distancesRight, true);
        findTransitNodes(distancesLeft, distancesRight, vs, distancesToNearestTransitNode);
    }

    for (int sweepIndexY = 0; sweepIndexY < gridsize; sweepIndexY++) {
        std::vector<DistanceData> distancesLeft;
        std::vector<DistanceData> distancesRight;

        std::vector<int> vs;
        std::unordered_map<int, std::vector<NodeDistance>> distancesToNearestTransitNode;

        //std::vector<std::vector<std::pair<int, std::unordered_map<int, int>>>> distancesVertical(gridsize);
        std::vector<std::array<std::vector<std::pair<int, std::unordered_map<int, int>>>, 2>> distancesHorizontal(gridsize);

        std::array<std::unordered_map<int, int>, 2> nodeIdxToMapIdxVertical;

        for (int sweepIndexX = 0; sweepIndexX < gridsize; sweepIndexX++) {
            for (int edgeIndex = 0; edgeIndex < edgeBucketsHorizontal[sweepIndexX][sweepIndexY].size(); edgeIndex++) {
                int vIndex = graph->sources[edgeBucketsHorizontal[sweepIndexX][sweepIndexY][edgeIndex]];
                std::vector<NodeDistance> dijkstraResults = processSingleNodeVertical(sweepIndexX, sweepIndexY, vIndex, distancesLeft, distancesRight, nodeIdxToMapIdxVertical);
                distancesToNearestTransitNode[vIndex] = dijkstraResults;
                vs.push_back(vIndex);
            }
            findTransitNodes(distancesLeft, distancesRight, vs, distancesToNearestTransitNode);
        }
    }

    computeDistancesBetweenTransitNodes();
    int x = 3;
}

void TransitNodesRouting::findTransitNodes(std::vector<DistanceData> &distancesLeft, std::vector<DistanceData> &distancesRight, std::vector<int> &vs, std::unordered_map<int, std::vector<NodeDistance>> &distancesToNearestTransitNode) {
    std::unordered_set<int> transitNodesPre;
    for (int i = 0; i < distancesLeft.size(); i++) {
        for (int j = 0; j < distancesRight.size(); j++) {
            int minDist = 2000000000;
            int minVIndex = -1;
            for (int k = 0; k < vs.size(); k++) {
                int vIndex = vs[k];
                // node does not exist in one of the tables -> skip
                if (distancesLeft[i].distanceToV.find(vIndex) == distancesLeft[i].distanceToV.end() ||
                distancesRight[j].distanceToV.find(vIndex) == distancesRight[j].distanceToV.end())
                    continue;
                int distToLeft = distancesLeft[i].distanceToV.at(vIndex);
                int distToRight = distancesRight[j].distanceToV.at(vIndex);
                if (distToLeft + distToRight < minDist) {
                    minDist = distToLeft + distToRight;
                    minVIndex = vIndex;
                }
            }
            if (minVIndex != -1) {
                transitNodesPre.insert(minVIndex);
            }
        }
    }

    for (auto& tn : transitNodesPre) {
        transitNodes.push_back(tn);
        std::vector<NodeDistance> distancesToTN = distancesToNearestTransitNode.at(tn);
        for (int j = 0; j < distancesToTN.size(); j++) {
            int uIndex = distancesToTN[j].nodeIndex;
            int dist = distancesToTN[j].distanceToV;
            localTransitNodes[uIndex].push_back(NodeDistance(uIndex, dist));
        }
    }
}

bool TransitNodesRouting::sameCell(Vec2Sphere &v1, Vec2Sphere &v2) {
    Vec2 v1_t = Vec2(v1);
    Vec2 v2_t = Vec2(v2);
    if (std::floor(v1_t.x * gridsize) == std::floor(v2_t.x * gridsize) &&
        std::floor(v1_t.y * gridsize) == std::floor(v2_t.y * gridsize)) 
        return true;
    return false;
}

// this should be trivial to run concurrently
void TransitNodesRouting::computeDistancesBetweenTransitNodes() {
    transitNodesDistances = std::vector<std::vector<int>> (transitNodes.size(), std::vector<int> (transitNodes.size()));
    for (int i = 0; i < transitNodes.size(); i++) {
        std::vector<int> distances = dijkstraSSSP(transitNodes[i]);
        for (int j = 0; j < transitNodes.size(); j++) {
            transitNodesDistances[i][j] = distances[transitNodes[j]];
        }
    }
}

// TODO: There are now 3 different dijkstra implementation, try to refactor them into two or less
std::vector<int> TransitNodesRouting::dijkstraSSSP(int source) {
    std::vector<int> dist;
    std::vector<int> prev; 
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;
    for (int i = 0; i < graph->nodes.size(); i++) {
        int max_int = 2147483647;
        dist.push_back(max_int);
        prev.push_back(-1);
    }
    dist[source] = 0;
    pq.push(std::make_pair(0, source));
    std::vector<bool> explored (graph->nodes.size(), false);

    while (!pq.empty()) {
        std::pair<int, int> node = pq.top();
        pq.pop();
        int u = node.second;
        if (explored[u])
            continue;
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
    return dist;
}

// modified dijkstra which stops after all boundaryNodes have been explored
std::vector<NodeDistance> TransitNodesRouting::dijkstra(int startIndex, std::vector<BoundaryNodeData> &boundaryNodes, std::vector<NodeDistance> &cLeft, std::vector<NodeDistance> &cRight, int n_boundaryNodes) {
    // TODO: store distances (maybe return dist array?)
    std::vector<int> prev;
    std::vector<int> dist;
    std::vector<NodeDistance> nodeDistances;
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
        if (boundaryNodes[u].isAtCellBoundary) {
            counter++;
            if (boundaryNodes[u].relPos == RelativePosition::LEFTLOWER)
                cLeft[boundaryNodes[u].localIndex].distanceToV = dist[u];
            else if (boundaryNodes[u].relPos == RelativePosition::RIGHTUPPER)
                cRight[boundaryNodes[u].localIndex].distanceToV = dist[u]; 
            boundaryNodes[u].isAtCellBoundary = false;
        }
        if (sameCell(graph->nodes[u], graph->nodes[startIndex]))
            nodeDistances.push_back(NodeDistance(u, dist[u]));
        
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
    return nodeDistances;
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
            for (int i = 0; i < edgeBucketsHorizontal[x][y].size(); i++) {
                nodes.push_back(graph->nodes[graph->sources[edgeBucketsHorizontal[x][y][i]]]);
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