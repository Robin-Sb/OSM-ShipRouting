#include "TransitNodesRouting.h"

TransitNodesRouting::TransitNodesRouting(std::shared_ptr<Graph> _graph, int _gridsize) {
    graph = _graph; 
    gridsize = _gridsize;
}

// main sweepline algorithm
TransitNodesData TransitNodesRouting::sweepLineTransitNodesMain() {
    localTransitNodes = std::vector<std::vector<NodeDistance>> (graph->nodes.size());
    transitNodesOfCells = std::vector<std::vector<std::unordered_set<int>>> (gridsize, std::vector<std::unordered_set<int>> (gridsize));

    std::shared_ptr<std::vector<std::vector<std::vector<int>>>> ebv_ptr = std::make_shared<std::vector<std::vector<std::vector<int>>>>(edgeBucketsVertical);
    std::shared_ptr<std::vector<std::vector<std::vector<int>>>> ebh_ptr = std::make_shared<std::vector<std::vector<std::vector<int>>>>(edgeBucketsHorizontal);

    // run a sweepline over all cells in x direction
    // store transit nodes and distances of all corresponding nodes
    for (int sweepIndexX = 0; sweepIndexX < gridsize; sweepIndexX++) {
        SingleTnPass tnSearchAlgo = SingleTnPass(sweepIndexX, 0, ebv_ptr, ebh_ptr, graph, gridsize, true);
        tnSearchAlgo.singleSweepLinePass();
        tnSearchAlgo.findTransitNodes(transitNodesOfCells, transitNodeTmp);
        tnSearchAlgo.assignDistances(transitNodeTmp, transitNodesOfCells, localTransitNodes);
        std::cout << "x: " << sweepIndexX << " finished \n";
    }

    for (int sweepIndexY = 0; sweepIndexY < gridsize; sweepIndexY++) {
        SingleTnPass tnSearchAlgo = SingleTnPass(0, sweepIndexY, ebh_ptr, ebv_ptr, graph, gridsize, false);
        tnSearchAlgo.singleSweepLinePass();
        tnSearchAlgo.findTransitNodes(transitNodesOfCells, transitNodeTmp);
        tnSearchAlgo.assignDistances(transitNodeTmp, transitNodesOfCells, localTransitNodes);
        std::cout << "y: " << sweepIndexY << " finished \n";
    }

    collectTransitNodes();
    std::cout << "TNs collected \n";
    computeDistancesBetweenTransitNodes();
    std::cout << "Distances computed \n";
    TransitNodesData transitNodesData = postprocessTransitNodes();
    return transitNodesData;
}

// convert transit nodes from set to array
void TransitNodesRouting::collectTransitNodes() {
    transitNodes = std::vector<int> (transitNodeTmp.size());
    for (auto& tn : transitNodeTmp) {
        transitNodes[tn.second] = tn.first;
    }
}

// convert transit nodes in a way so that they are easier to process
TransitNodesData TransitNodesRouting::postprocessTransitNodes() {
    // convert to array instead of set for consistent ordering
    std::vector<std::vector<std::vector<int>>> transitNodesPerCell (gridsize, std::vector<std::vector<int>> (gridsize));
    for (int x = 0; x < gridsize; x++) {
        for (int y = 0; y < gridsize; y++) {
            for (auto &i : transitNodesOfCells[x][y]) {
                transitNodesPerCell[x][y].push_back(i);
            }
        }
    }

    std::vector<std::vector<int>> distancesToLocalTransitNodes (graph->nodes.size());
    for (int i = 0; i < localTransitNodes.size(); i++) {
        int cellX = UtilFunctions::getCellX(graph->nodes[i], gridsize);
        int cellY = UtilFunctions::getCellY(graph->nodes[i], gridsize);
        std::vector<int> transitNodeDistancesOfSingleNode (transitNodesPerCell[cellX][cellY].size(), 2000000000);
        for (int k = 0; k < transitNodesPerCell[cellX][cellY].size(); k++) {
            for (int j = 0; j < localTransitNodes[i].size(); j++) {
                // localTransitNodes[i] contains some nodes two times -> we could just break, but try to find the issue first
                if (localTransitNodes[i][j].nodeIndex == transitNodesPerCell[cellX][cellY][k]) {
                    transitNodeDistancesOfSingleNode[k] = localTransitNodes[i][j].distanceToV;
                }
            }
        }
        distancesToLocalTransitNodes[i] = transitNodeDistancesOfSingleNode;
    }
    return TransitNodesData(transitNodes, transitNodesDistances, transitNodesPerCell, distancesToLocalTransitNodes, gridsize, gridsize);
}

void TransitNodesRouting::computeDistancesBetweenTransitNodes() {
    transitNodesDistances = std::vector<std::vector<int>> (transitNodes.size(), std::vector<int> (transitNodes.size()));
    for (int i = 0; i < transitNodes.size(); i++) {
        std::vector<int> distances = dijkstraSSSP(transitNodes[i]);
        for (int j = 0; j < transitNodes.size(); j++) {
            transitNodesDistances[i][j] = distances[transitNodes[j]];
        }
    }
}

// single source shortest path dijkstra, used to calculate distances between transit nodes
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

            int altDist = dist[u] + graph->costs[i];
            if (altDist < dist[v]) {
                dist[v] = altDist;
                prev[v] = u;
                pq.push(std::make_pair(dist[v], v));
            }
        }
    }
    return dist;
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

    // antimeridian case, somewhat hacky but the problem is also not exactly well defined
    if (lastIndexX - firstIndexX > gridsize/2) {
        int firstIndexTmp = firstIndexX;
        firstIndexX = lastIndexX % gridsize;
        lastIndexX = firstIndexTmp;
        if (firstIndexX != 0)
            lastIndexX + gridsize;
    }
    for (int counter = 0; firstIndexX + counter < lastIndexX; counter++) {
        edgeBucketsVertical[(firstIndexX + counter) % gridsize][yIndex].push_back(edgeIndex);
    }
}

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

// find all cell crossings of edges in both vertical and horizontal direction, store them
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