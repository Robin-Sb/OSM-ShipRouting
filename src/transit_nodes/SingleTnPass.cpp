#include "SingleTnPass.h"

SingleTnPass::SingleTnPass(int _sweepIndexX, int _sweepIndexY, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> _edgeBucketsMain, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> _edgeBucketsSecondary, std::shared_ptr<Graph> _graph, int _gridsize, bool _vertical) {
    sweepIndexX = _sweepIndexX;
    sweepIndexY = _sweepIndexY;
    edgeBucketsMain = _edgeBucketsMain;
    edgeBucketsSecondary = _edgeBucketsSecondary;
    graph = _graph;
    gridsize = _gridsize;
    vertical = _vertical;
}

// main function of a single sweepline pass
// looks at a single node v, which is the node with smaller id of an edge which crosses two gridcells
// for this node v, the algorithm looks at five gridcells which are two gridcells left and right (or up and down) of v
// of those 5 gridcells each left and right, all shortest paths between the nodes on the left and on the right are computed
// if v is on one of these shortest paths, v becomes a transit node and the corresponding cell is associated with v
// for every node inside the corresponding cell, the distance to v is stored
void SingleTnPass::singleSweepLinePass() {
    for (int &iterableIndex = vertical ? sweepIndexY : sweepIndexX; iterableIndex < gridsize; iterableIndex++) {
        for (int edgeIndex = 0; edgeIndex < edgeBucketsMain->at(sweepIndexX)[sweepIndexY].size(); edgeIndex++) {
            int edgeIndexGlobal = edgeBucketsMain->at(sweepIndexX)[sweepIndexY][edgeIndex];
            // always use the node with smaller id
            vIndex = std::min(graph->targets[edgeIndexGlobal], graph->sources[edgeIndexGlobal]);
            if (vIndex == 5068 || vIndex == 6932)   
                int x = 3;
            // skip if already computed
            if (vs.find(vIndex) != vs.end())
                continue;

            // reset data
            cNegative.clear();
            cPositive.clear();
            boundaryNodes.clear();

            // checks all nodes two grid cells left and right and computes dijkstra in 3x3 range
            std::vector<NodeDistance> dijkstraResults = processSingleNode(vIndex);
            distancesToNearestTransitNode[vIndex] = dijkstraResults;
            vs.insert(vIndex);
        }
       std::cout << "x: " << sweepIndexX << ", y: " << sweepIndexY << "\n";
    }
}

std::vector<NodeDistance> SingleTnPass::processSingleNode(int vIndex) {
    boundaryNodes = std::vector<BoundaryNodeData> (graph->nodes.size());
    // offset in the vertical case incase we are at the edge of grid (in y direction)
    int minCell = vertical ? std::max(0, 2 - sweepIndexY) - 2 : -2;
    int maxCell = vertical ?  std::min((gridsize - 1) - sweepIndexY, 2) : 2;
    
    for (int i = minCell; i <= maxCell; i++) {
        // look both at right and left boundary of cell (or up and down)
        for (int cell = 0; cell < 2; cell++) {
            if (vertical)
                searchVertical(i, cell, true);
            else 
                searchHorizontal(i, cell, true);
        }
    }
    // once again, edge of grid case
    int maxCellOrthogonal = vertical ? std::min(maxCell + 1, (gridsize - 1) - sweepIndexY) : 3;
    // take care of the nodes which cross cells horizontally
    for (int i = minCell; i <= maxCellOrthogonal; i++) {
        if (vertical)
            searchVertical(i, 0, false);
        else 
            searchHorizontal(i, 0, false);
    }
    // run a 3x3 dijkstra and store the results
    std::vector<NodeDistance> dijkstraResults = dijkstra();
    storeDistancesNegative();
    storeDistancesPositive();
    return dijkstraResults;
}

// find all boundary nodes two cells left and right 
void SingleTnPass::searchVertical(int i, int cell, bool verticalPass) {
    // special case caught due to minY and maxY
    int yIndex = sweepIndexY + i;
    // calculate x gridcell to left and to the right
    // also includes antimeridian wraparound
    int xIndexL = (sweepIndexX + cell - 3 + gridsize) % gridsize;
    int xIndexR = (sweepIndexX + cell + 2) % gridsize; 
    std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets = verticalPass ? edgeBucketsMain : edgeBucketsSecondary;
    findBoundaryNodesNegative(xIndexL, yIndex, verticalPass, edgeBuckets);
    findBoundaryNodesPositive(xIndexR, yIndex, verticalPass, edgeBuckets);
}

// find all boundary nodes two cells up and down 
// horizontal and vertical case are treated differently, because otherwise the algorithm became relatively difficult to maintain
void SingleTnPass::searchHorizontal(int i, int cell, bool horizontalPass) {
    // special case caught due to minY and maxY
    int yIndexPositive = horizontalPass ? sweepIndexY - cell + 3 : sweepIndexY - cell + 2;
    int yIndexNegative = horizontalPass ? sweepIndexY - cell - 2 : sweepIndexY - cell - 3;
    // wraparound incase x becomes negative
    int xIndex = (sweepIndexX + i + gridsize) % gridsize;
    std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets = horizontalPass ? edgeBucketsMain : edgeBucketsSecondary;
    findBoundaryNodesNegative(xIndex, yIndexNegative, horizontalPass, edgeBuckets);
    findBoundaryNodesPositive(xIndex, yIndexPositive, horizontalPass, edgeBuckets);
}


// find boundary nodes to the left (or down) at a certain cell position
void SingleTnPass::findBoundaryNodesNegative(int xIndex, int yIndex, bool verticalPass, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets) {
    for (int j = 0; j < edgeBuckets->at(xIndex)[yIndex].size(); j++) {
        int edgeIndex = edgeBuckets->at(xIndex)[yIndex][j];
        int startIndex = graph->sources[edgeIndex];
        int endIndex = graph->targets[edgeIndex];
        // the edge in the opposite direction was found -> skip this one
        if (boundaryNodes[startIndex].isAtCellBoundary) {
            boundaryNodes[startIndex].edges.push_back(edgeIndex);
            continue;
        } 
        if (boundaryNodes[endIndex].isAtCellBoundary) {
            boundaryNodes[endIndex].edges.push_back(edgeIndex);
            continue;
        }
        // take vertex with minimum index for uniqueness
        int nodeIndex = std::min(startIndex, endIndex); 
        // store empty entry for all nodes at cell boundary
        if (nodeIdxToMapIdxNegative.find(nodeIndex) == nodeIdxToMapIdxNegative.end()) {
            std::unordered_map<int, int> distanceData;
            nodeIdxToMapIdxNegative[nodeIndex] = distancesNegative.size();
            distancesNegative.push_back(DistanceData(nodeIndex, distanceData, Vec2(xIndex, yIndex)));
        }
        // store for nodeIndex that it is a cell boundary, including its position in cNegative
        boundaryNodes[nodeIndex] = BoundaryNodeData(true, cNegative.size(), RelativePosition::NEGATIVE);
        boundaryEdges.insert(std::pair<int, std::vector<int>>(nodeIndex, std::vector<int> {edgeIndex}));
        // push back one entry for every node with distance c
        cNegative.push_back(NodeDistance(nodeIndex, 0));
    }
}

// works the same as negative, but in positive x or y direction
void SingleTnPass::findBoundaryNodesPositive(int xIndex, int yIndex, bool verticalPass, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets) {
    for (int j = 0; j < edgeBuckets->at(xIndex)[yIndex].size(); j++) {
        int edgeIndex = edgeBuckets->at(xIndex)[yIndex][j];
        int startIndex = graph->sources[edgeIndex];
        int endIndex = graph->targets[edgeIndex];
        // another edge with same node on cell boundary -> add to edges
        if (boundaryNodes[startIndex].isAtCellBoundary) {
            boundaryEdges.at(startIndex).push_back(edgeIndex);
            continue;
        } 
        if (boundaryNodes[endIndex].isAtCellBoundary) {
            boundaryEdges.at(endIndex).push_back(edgeIndex);
            continue;
        }
        int nodeIndex = std::min(startIndex, endIndex); 
        if (nodeIdxToMapIdxPositive.find(nodeIndex) == nodeIdxToMapIdxPositive.end()) {
            std::unordered_map<int, int> distanceData;
            nodeIdxToMapIdxPositive[nodeIndex] = distancesPositive.size();
            distancesPositive.push_back(DistanceData(nodeIndex, distanceData, Vec2(xIndex, yIndex)));
        }
        boundaryNodes[nodeIndex] = BoundaryNodeData(true, cPositive.size(), RelativePosition::POSITIVE);
        boundaryEdges.insert(std::pair<int, std::vector<int>>(nodeIndex, std::vector<int> {edgeIndex}));
        cPositive.push_back(NodeDistance(nodeIndex, 2147483647));
    }
}

// store distances of the dijkstra run between v and every node on the cell boundary
void SingleTnPass::storeDistancesPositive() {
    for (int j = 0; j < cPositive.size(); j++) {
        int nodeIndex = cPositive[j].nodeIndex;
        int mapIndex;

        mapIndex = nodeIdxToMapIdxPositive.find(nodeIndex)->second;
        distancesPositive[mapIndex].distanceToV.insert(std::make_pair(vIndex, cPositive[j].distanceToV));
    }
}

void SingleTnPass::storeDistancesNegative() {
    for (int j = 0; j < cNegative.size(); j++) {
        int nodeIndex = cNegative[j].nodeIndex;
        int mapIndex;

        mapIndex = nodeIdxToMapIdxNegative.find(nodeIndex)->second;
        distancesNegative[mapIndex].distanceToV.insert(std::make_pair(vIndex, cNegative[j].distanceToV));
    }
}

// standard dijkstra except that it stops after exploring 3x3 grid range and also stores distances to boundary nodes
std::vector<NodeDistance> SingleTnPass::dijkstra() {
    std::vector<int> prev;
    std::vector<int> dist;
    std::vector<NodeDistance> nodeDistances;
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;
    for (int i = 0; i < graph->nodes.size(); i++) {
        int max_int = 2147483647;
        dist.push_back(max_int);
        prev.push_back(-1);
    }
    dist[vIndex] = 0;
    pq.push(std::make_pair(0, vIndex));
    std::vector<bool> explored (graph->nodes.size(), false);
    int counter = 0;

    while (!pq.empty()) {
        std::pair<int, int> node = pq.top();
        pq.pop();
        int u = node.second;
        if (explored[u])
            continue;
        if (boundaryNodes[u].isAtCellBoundary) {
            if (boundaryNodes[u].relPos == RelativePosition::NEGATIVE)
                cNegative[boundaryNodes[u].localIndex].distanceToV = dist[u];
            else if (boundaryNodes[u].relPos == RelativePosition::POSITIVE)
                cPositive[boundaryNodes[u].localIndex].distanceToV = dist[u]; 
            boundaryNodes[u].isAtCellBoundary = false;
        }
        // store distance of v to all settled nodes
        nodeDistances.push_back(NodeDistance(u, dist[u]));
        
        explored[u] = true;
        for (int i = graph->offsets[u]; i < graph->offsets[u + 1]; i++) {
            int v = graph->targets[i];
            if (explored[v])
                continue;

            int vCellX = UtilFunctions::getCellX(graph->nodes[v], gridsize);
            int vCellY = UtilFunctions::getCellY(graph->nodes[v], gridsize);
            // stop if outside of 3x3 (kind of) search radius
            if (std::abs(vCellX - sweepIndexX) > 3 && std::abs(vCellY - sweepIndexY) > 3)
                continue;

            int altDist = dist[u] + graph->costs[i];
            if (altDist < dist[v]) {
                dist[v] = altDist;
                prev[v] = u;
                pq.push(std::make_pair(dist[v], v));
            }
        }
    }
    return nodeDistances;
}

// looks at all boundary nodes to the left and to the right (or up and down) of a single sweepline pass
// then checks for every two nodes, whether those two nodes are 4 grid cells apart in the other direction
// if they are, check for every node on the sweepline, through which v the shortest path passes through
// v then becomes a transit node of the two cells corresponding to the two nodes
void SingleTnPass::findTransitNodes(std::vector<std::vector<std::unordered_set<int>>> &transitNodesOfCells, std::unordered_map<int, int> &transitNodeTmp) {
    for (int i = 0; i < distancesNegative.size(); i++) {
        for (int j = 0; j < distancesPositive.size(); j++) {
            DistanceData vL = distancesNegative[i];
            DistanceData vR = distancesPositive[j];
            
            int gridCellNegative = vertical ? vL.cell.y : vL.cell.x; 
            int gridCellPositive = vertical ? vR.cell.y : vR.cell.x; 

            // if distance bigger than 4 gridcells, skip
            // second part is antimeridian case
            if (std::abs(gridCellNegative - gridCellPositive) > 4 && (std::min(gridCellNegative, gridCellPositive) + gridsize) - std::max(gridCellNegative, gridCellPositive) > 4)
                continue;

            int minDist = 2000000000;
            int minVIndex = -1;
            // find v which leads to the minimal distance
            for (auto &vIndex : vs) {
                // node does not exist in one of the tables -> skip
                if (vL.distanceToV.find(vIndex) == vL.distanceToV.end() ||
                vR.distanceToV.find(vIndex) == vR.distanceToV.end())
                    continue;
                int distToLeft = vL.distanceToV.at(vIndex);
                int distToRight = vR.distanceToV.at(vIndex);
                if (distToLeft + distToRight < minDist) {
                    minDist = distToLeft + distToRight;
                    minVIndex = vIndex;
                }
            }
            // no transit node found -> skip
            if (minVIndex == -1) 
                continue;

            int transitNodeId; 
            // if v wasn't a transit node yet, append it to transit nodes
            if (transitNodeTmp.find(minVIndex) == transitNodeTmp.end()) {
                transitNodeId = transitNodeTmp.size();
                transitNodeTmp[minVIndex] = transitNodeId;
            } 
            // if it does exist already, just use the old id
            else {
                transitNodeId = transitNodeTmp.at(minVIndex);
            }

            auto getCell = vertical ? UtilFunctions::getCellY : UtilFunctions::getCellX;
            // add the transit node to all cells the node is part of
            // in almost all cases, this will only be the two cells of of one edge
            for (int k = 0; k < boundaryEdges.at(vL.referenceNodeIndex).size(); k++) {
                int sourceNeg = graph->sources[boundaryEdges.at(vL.referenceNodeIndex)[k]];
                int targetNeg = graph->targets[boundaryEdges.at(vL.referenceNodeIndex)[k]];
                int srcCellXNeg = getCell(graph->nodes[sourceNeg], gridsize);
                int trgCellXNeg = getCell(graph->nodes[targetNeg], gridsize);
                if (vertical) {
                    transitNodesOfCells[(sweepIndexX - 3 + gridsize) % gridsize][getCell(graph->nodes[sourceNeg], gridsize)].insert(transitNodeId);
                    transitNodesOfCells[(sweepIndexX - 3 + gridsize) % gridsize][getCell(graph->nodes[targetNeg], gridsize)].insert(transitNodeId);
                } else {
                    transitNodesOfCells[getCell(graph->nodes[sourceNeg], gridsize)][(sweepIndexY - 3 + gridsize) % gridsize].insert(transitNodeId);
                    transitNodesOfCells[getCell(graph->nodes[targetNeg], gridsize)][(sweepIndexY - 3 + gridsize) % gridsize].insert(transitNodeId);
                }
            }

            // same as above, but for the nodes on the right
            for (int k = 0; k < boundaryEdges.at(vR.referenceNodeIndex).size(); k++) {
                int sourcePos = graph->sources[boundaryEdges.at(vR.referenceNodeIndex)[k]];
                int targetPos = graph->targets[boundaryEdges.at(vR.referenceNodeIndex)[k]];
                int srcCellXPos = getCell(graph->nodes[sourcePos], gridsize);
                int trgCellXPos = getCell(graph->nodes[targetPos], gridsize);
                if (vertical) {
                    transitNodesOfCells[(sweepIndexX + 2) % gridsize][getCell(graph->nodes[sourcePos], gridsize)].insert(transitNodeId);
                    transitNodesOfCells[(sweepIndexX + 2) % gridsize][getCell(graph->nodes[targetPos], gridsize)].insert(transitNodeId);
                } else {
                    transitNodesOfCells[getCell(graph->nodes[sourcePos], gridsize)][(sweepIndexY + 2) % gridsize].insert(transitNodeId);
                    transitNodesOfCells[getCell(graph->nodes[targetPos], gridsize)][(sweepIndexY + 2) % gridsize].insert(transitNodeId);
                }
            }
        }
    }
}

// for all transit nodes which are associated to a cell,
// store the distance from every node in the cell to the transit node
void SingleTnPass::assignDistances(std::unordered_map<int, int> &transitNodeTmp, std::vector<std::vector<std::unordered_set<int>>> &transitNodesOfCells, std::vector<std::vector<NodeDistance>> &localTransitNodes) {
    // loop over all nodes which could be a transit node
    for (auto& potentialTn : distancesToNearestTransitNode) {
        int potentialTnIndex = potentialTn.first;
        // check if its a transit node
        if (transitNodeTmp.find(potentialTnIndex) != transitNodeTmp.end()) {
            int tnIndexLocal = transitNodeTmp.at(potentialTnIndex);
            // get the node, for which distances are computed to v
            std::vector<NodeDistance> correspondingNodes = potentialTn.second;
            for (int i = 0; i < correspondingNodes.size(); i++) {
                int nodeIndex = correspondingNodes[i].nodeIndex;
                // get the cell for this node
                int cellX = UtilFunctions::getCellX(graph->nodes[nodeIndex], gridsize);
                int cellY = UtilFunctions::getCellY(graph->nodes[nodeIndex], gridsize);
                // check if the transit node is actually a transit node of that cell
                if (transitNodesOfCells[cellX][cellY].find(tnIndexLocal) != transitNodesOfCells[cellX][cellY].end()) {
                    // if yes, add the distance to the node
                    localTransitNodes[nodeIndex].push_back(NodeDistance(tnIndexLocal, correspondingNodes[i].distanceToV));
                }
            }
        }
    }
}