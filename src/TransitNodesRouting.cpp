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

// TODO: wraparound antimeridian and pole case
void TransitNodesRouting::findTransitNodes() {
    std::array<int, 2> cellsLeft{-2, -1};
    std::array<int, 2> cellsRight{2, 3};

    // edgebuckets start at high value 
    // check why
    // did not figure out why, but seems correct
    for (int sweepIndexX = 0; sweepIndexX < gridsize; sweepIndexX++) {
        // [x_index][y_index][] 
        std::vector<std::vector<std::vector<int>>> distances (gridsize);
        for (int sweepIndexY = 0; sweepIndexY < gridsize; sweepIndexY++) {
            std::vector<int> potentialTransitNodes;
            for (int edgeIndex = 0; edgeIndex < edgeBucketsVertical[sweepIndexX][sweepIndexY].size(); edgeIndex++) {
                // just always use the source idk know actually 
                int vIndex = graph->sources[edgeBucketsVertical[sweepIndexX][sweepIndexY][edgeIndex]];
                Vec2 v = Vec2(graph->nodes[vIndex]);
                // push all the v into one array and then sort in the end by y-coordinate 
                potentialTransitNodes.push_back(vIndex);
                int gridCellX = std::floor(v.x * gridsize);
                int gridCellY = std::floor(v.y * gridsize);
                
                std::vector<std::pair<bool, std::pair<int,int>>> boundaryNodes (graph->nodes.size(), std::make_pair(false, std::make_pair(-1,-1)));
                std::vector<std::vector<int>> cLeft (5);

                // boundary condition incase we are the edge of the grid 
                int minY = std::max(0, 2 - gridCellY) - 2;
                int maxY = std::min((gridsize - 1) - gridCellY, 2);
                
                int n_boundaryNodes = 0;
                // this only processes the vertical nodes so far -> expand to horizontal
                for (int i = minY; i <= maxY; i++) {
                    int localIdx = i + 2;
                    for (int cell = 0; cell < 2; cell++) {
                        int yIndex = gridCellY + i;
                        // wraparound incase x becomes negative
                        int xIndex = ((gridCellX + (cell - 2)) + gridsize) % gridsize;
                        for (int j = 0; j < edgeBucketsVertical[xIndex][yIndex].size(); j++) {
                            // discard duplicate edges
                            int startIndex = graph->sources[edgeBucketsVertical[xIndex][yIndex][j]];
                            int endIndex = graph->targets[edgeBucketsVertical[xIndex][yIndex][j]];
                            if (boundaryNodes[startIndex].first || boundaryNodes[endIndex].first)
                                continue;
                            // always taking the first node here means sometimes the edge is included and sometimes it isn't
                            // I am not sure what the correct thing to do is here
                            boundaryNodes[startIndex] = std::make_pair(true, std::make_pair(localIdx, cLeft[localIdx].size()));
                            cLeft[localIdx].push_back(0); //graph->sources[edgeBucketsVertical[xIndex][yIndex][j]]);
                            n_boundaryNodes++;
                        }
                    }
                }
                dijkstra(vIndex, boundaryNodes, cLeft, n_boundaryNodes);
            }
        }
    }
}

// TODO: Finish
void TransitNodesRouting::dijkstra(int startIndex, std::vector<std::pair<bool, std::pair<int,int>>> boundaryNodes, std::vector<std::vector<int>> cLeft, int n_boundaryNodes) {
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
            cLeft[boundaryNodes[u].second.first][boundaryNodes[u].second.second] = dist[u];
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
    // float smallerV = std::min<float>(start.x, end.x);
    // float biggerV = std::max<float>(start.x, end.x);
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
    if (start.y < end.y) {
        smallerY = end.y;
        biggerY = start.y;
        xIndex = std::floor(start.x * gridsize);
    } else {
        smallerY = start.y;
        biggerY = end.y;
        xIndex = std::floor(end.x * gridsize);
    }
    int firstIndexY = std::ceil(smallerY * gridsize);
    for (int counter = 0; firstIndexY + counter < biggerY * gridsize; counter++) {
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