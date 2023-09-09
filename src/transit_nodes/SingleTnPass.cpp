#include "SingleTnPass.h"

SingleTnPass::SingleTnPass(int _sweepIndexX, int _sweepIndexY, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> _edgeBucketsMain, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> _edgeBucketsSecondary, std::shared_ptr<Graph> _graph, int _gridsize, bool _vertical) {
    sweepIndexX = _sweepIndexX;
    sweepIndexY = _sweepIndexY;
    edgeBucketsMain = _edgeBucketsMain;
    edgeBucketsSecondary = _edgeBucketsSecondary;
    graph = _graph;
    gridsize = _gridsize;
    vertical = _vertical;
    n_boundaryNodes = 0;
}

void SingleTnPass::singleSweepLinePass() {
    //int& iterableIndex = sweepIndexY; 
    for (int &iterableIndex = vertical ? sweepIndexY : sweepIndexX; iterableIndex < gridsize; iterableIndex++) {
        for (int edgeIndex = 0; edgeIndex < edgeBucketsMain->at(sweepIndexX)[sweepIndexY].size(); edgeIndex++) {
            vIndex = graph->sources[edgeBucketsMain->at(sweepIndexX)[sweepIndexY][edgeIndex]];

            // skip duplicates
            if (vs.find(vIndex) != vs.end())
                continue;

            cNegative.clear();
            cPositive.clear();
            boundaryNodes.clear();

            std::vector<NodeDistance> dijkstraResults = processSingleNode(vIndex);
            distancesToNearestTransitNode[vIndex] = dijkstraResults;
            vs.insert(vIndex);
        }
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
    int maxCellOrthogonal = vertical ? std::min(maxCell + 1, (gridsize - 1) - sweepIndexY) : 3;
    for (int i = minCell; i <= maxCellOrthogonal; i++) {
        if (vertical)
            searchVertical(i, 0, false);
        else 
            searchHorizontal(i, 0, false);
    }
    std::vector<NodeDistance> dijkstraResults = dijkstra();
    storeDistancesNegative();
    storeDistancesPositive();
    return dijkstraResults;
}

void SingleTnPass::searchVertical(int i, int cell, bool verticalPass) {
    // special case caught due to minY and maxY
    int yIndex = sweepIndexY + i;
    // wraparound incase x becomes negative
    int xIndexL = (sweepIndexX + cell - 3 + gridsize) % gridsize;
    int xIndexR = (sweepIndexX + cell + 2) % gridsize; 
    std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets = verticalPass ? edgeBucketsMain : edgeBucketsSecondary;
    findBoundaryNodesNegative(xIndexL, yIndex, verticalPass, edgeBuckets);
    findBoundaryNodesPositive(xIndexR, yIndex, verticalPass, edgeBuckets);
}

void SingleTnPass::searchHorizontal(int i, int cell, bool horizontalPass) {
    // special case caught due to minY and maxY
    int yIndexPositive = sweepIndexY - cell + 3;
    int yIndexNegative = sweepIndexY - cell - 2;
    // wraparound incase x becomes negative
    int xIndex = (sweepIndexX + i + gridsize) % gridsize;
    std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets = horizontalPass ? edgeBucketsMain : edgeBucketsSecondary;
    findBoundaryNodesNegative(xIndex, yIndexNegative, horizontalPass, edgeBuckets);
    findBoundaryNodesPositive(xIndex, yIndexPositive, horizontalPass, edgeBuckets);
}


void SingleTnPass::findBoundaryNodesNegative(int xIndex, int yIndex, bool verticalPass, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets) {
    for (int j = 0; j < edgeBuckets->at(xIndex)[yIndex].size(); j++) {
        int startIndex = graph->sources[edgeBuckets->at(xIndex)[yIndex][j]];
        int endIndex = graph->targets[edgeBuckets->at(xIndex)[yIndex][j]];
        // the edge in the opposite direction was found -> skip this one
        if (boundaryNodes[startIndex].isAtCellBoundary || boundaryNodes[endIndex].isAtCellBoundary)
            continue;
        // order nodes by their x/y coordinate and take the far away node (because then all nodes within cell will be setlled)
        int nodeIndex = orderNodes(startIndex, endIndex, verticalPass).second;
        if (nodeIdxToMapIdxNegative.find(nodeIndex) == nodeIdxToMapIdxNegative.end()) {
            std::unordered_map<int, int> distanceData;
            nodeIdxToMapIdxNegative[nodeIndex] = distancesNegative.size();
            distancesNegative.push_back(DistanceData(nodeIndex, distanceData, Vec2(xIndex, yIndex)));
        }
        boundaryNodes[nodeIndex] = BoundaryNodeData(true, cNegative.size(), RelativePosition::LEFTLOWER);
        cNegative.push_back(NodeDistance(nodeIndex, 0));
        n_boundaryNodes++;
    }
}

void SingleTnPass::findBoundaryNodesPositive(int xIndex, int yIndex, bool verticalPass, std::shared_ptr<std::vector<std::vector<std::vector<int>>>> edgeBuckets) {
    for (int j = 0; j < edgeBuckets->at(xIndex)[yIndex].size(); j++) {
        int startIndex = graph->sources[edgeBuckets->at(xIndex)[yIndex][j]];
        int endIndex = graph->targets[edgeBuckets->at(xIndex)[yIndex][j]];
        // the edge in the opposite direction was found -> skip this one
        if (boundaryNodes[startIndex].isAtCellBoundary || boundaryNodes[endIndex].isAtCellBoundary)
            continue;
        // order nodes by their x/y coordinate and take the far away node (because then all nodes within cell will be setlled)
        int nodeIndex = orderNodes(startIndex, endIndex, verticalPass).second;
        if (nodeIdxToMapIdxPositive.find(nodeIndex) == nodeIdxToMapIdxPositive.end()) {
            std::unordered_map<int, int> distanceData;
            nodeIdxToMapIdxPositive[nodeIndex] = distancesPositive.size();
            distancesPositive.push_back(DistanceData(nodeIndex, distanceData, Vec2(xIndex, yIndex)));
        }
        boundaryNodes[nodeIndex] = BoundaryNodeData(true, cPositive.size(), RelativePosition::RIGHTUPPER);
        cPositive.push_back(NodeDistance(nodeIndex, 0));
        n_boundaryNodes++;
    }
}

std::pair<int, int> SingleTnPass::orderNodes(int startIndex, int endIndex, bool verticalPass) {
    // always use the node which is outside of the cell, s.t. a dijkstra run settles all nodes in the cells
    if (verticalPass) {
        int startCellX = UtilFunctions::getCellX(graph->nodes[startIndex], gridsize);
        int endCellX = UtilFunctions::getCellX(graph->nodes[endIndex], gridsize);
        return startCellX > endCellX ? std::make_pair(startIndex, endIndex) : std::make_pair(endIndex, startIndex);
    } else {
        int startCellY = UtilFunctions::getCellY(graph->nodes[startIndex], gridsize);
        int endCellY = UtilFunctions::getCellY(graph->nodes[endIndex], gridsize);
        return startCellY > endCellY ? std::make_pair(startIndex, endIndex) : std::make_pair(endIndex, startIndex);
    }
}

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
        if (counter == n_boundaryNodes) 
            break;
        if (boundaryNodes[u].isAtCellBoundary) {
            counter++;
            if (boundaryNodes[u].relPos == RelativePosition::LEFTLOWER)
                cNegative[boundaryNodes[u].localIndex].distanceToV = dist[u];
            else if (boundaryNodes[u].relPos == RelativePosition::RIGHTUPPER)
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

void SingleTnPass::findTransitNodes(std::vector<std::vector<std::unordered_set<int>>> &transitNodesOfCells, std::unordered_map<int, int> &transitNodeTmp) {
    for (int i = 0; i < distancesNegative.size(); i++) {
        for (int j = 0; j < distancesPositive.size(); j++) {
            DistanceData vL = distancesNegative[i];
            DistanceData vR = distancesPositive[j];
            auto getCell = vertical ? UtilFunctions::getCellY : UtilFunctions::getCellX;
            int gridCellNegative = vertical ? vL.cell.y : vL.cell.x; //getCell(graph->nodes[vL.referenceNodeIndex], gridsize);
            int gridCellPositive = vertical ? vR.cell.y : vR.cell.x; //getCell(graph->nodes[vR.referenceNodeIndex], gridsize);

            // if distance bigger than 4 gridcells, skip
            if (std::abs(gridCellNegative - gridCellPositive) > 4)
                continue;

            int minDist = 2000000000;
            int minVIndex = -1;
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
            // if transit node was not found yet, append it
            if (transitNodeTmp.find(minVIndex) == transitNodeTmp.end()) {
                transitNodeId = transitNodeTmp.size();
                transitNodeTmp[minVIndex] = transitNodeId;
            } 
            // if it does exist already, just use the old id
            else {
                transitNodeId = transitNodeTmp.at(minVIndex);
            }

            if (vertical) {
                transitNodesOfCells[sweepIndexX - 3][gridCellNegative].insert(transitNodeId);
                transitNodesOfCells[sweepIndexX + 2][gridCellPositive].insert(transitNodeId);
            } else {
                transitNodesOfCells[gridCellNegative][sweepIndexY - 3].insert(transitNodeId);
                transitNodesOfCells[gridCellPositive][sweepIndexY + 2].insert(transitNodeId);
            }

        }
    }
}

void SingleTnPass::assignDistances(std::unordered_map<int, int> &transitNodeTmp, std::vector<std::vector<std::unordered_set<int>>> &transitNodesOfCells, std::vector<std::vector<NodeDistance>> &localTransitNodes) {
    for (auto& potentialTn : distancesToNearestTransitNode) {
        int potentialTnIndex = potentialTn.first;
        if (transitNodeTmp.find(potentialTnIndex) != transitNodeTmp.end()) {
            int tnIndexLocal = transitNodeTmp.at(potentialTnIndex);
            std::vector<NodeDistance> correspondingNodes = potentialTn.second;
            for (int i = 0; i < correspondingNodes.size(); i++) {
                int nodeIndex = correspondingNodes[i].nodeIndex;
                int cellX = UtilFunctions::getCellX(graph->nodes[nodeIndex], gridsize);
                int cellY = UtilFunctions::getCellY(graph->nodes[nodeIndex], gridsize);
                // check if tnIndexLocal is a local transit node corresponding to that cell -> if yes, add distance
                if (transitNodesOfCells[cellX][cellY].find(tnIndexLocal) != transitNodesOfCells[cellX][cellY].end()) {
                    localTransitNodes[nodeIndex].push_back(NodeDistance(tnIndexLocal, correspondingNodes[i].distanceToV));
                }
            }
        }
    }
}