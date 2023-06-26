#include "TransitNodesRouting.h"

TransitNodesRouting::TransitNodesRouting(std::shared_ptr<Graph> _graph, int _gridsize) {
    graph = _graph; 
    gridsize = _gridsize;
}


// TODO: wraparound antimeridian and pole case
void TransitNodesRouting::findTransitNodes() {
    for (int sweepIndexX = 0; sweepIndexX < gridsize; sweepIndexX++) {
        for (int sweepIndexY = 0; sweepIndexY < gridsize; sweepIndexY++) {
            for (int edgeIndex = 0; edgeIndex < edgeBucketsVertical[sweepIndexX].size(); edgeIndex++) {
                // just always use the source idk know actually 
                Vec2Sphere v = graph->nodes[graph->sources[edgeBucketsVertical[sweepIndexX][sweepIndexY][edgeIndex]]];
                int gridCellX = std::floor(v.lon * gridsize);
                int gridCellY = std::floor(v.lat * gridsize);
                
                std::vector<bool> boundaryNodes (graph->nodes.size(), false);
                for (int i = -2; i <= 2; i++) {
                    std::array<int, 4> cells{-2, -1, 2, 3};
                    for (int cell = 0; cell < cells.size(); cell++) {
                        int yIndex = gridCellY + i;
                        int xIndex = gridCellX + cells[cell];
                        for (int j = 0; j < edgeBucketsVertical[xIndex][yIndex].size(); j++) {
                            boundaryNodes[graph->sources[edgeBucketsVertical[xIndex][yIndex][j]]] = true;
                        }
                    }
                }
                
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
        yIndex = std::floor(start.x * gridsize);
    } else {
        smallerX = start.x;
        biggerX = end.x;
        yIndex = std::floor(end.x * gridsize);
    }
    // float smallerV = std::min<float>(start.x, end.x);
    // float biggerV = std::max<float>(start.x, end.x);
    int firstIndexX = std::ceil(smallerX * gridsize);
    for (int counter = 0; firstIndexX + counter < biggerX * gridsize; counter++) {
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
        xIndex = std::floor(start.y * gridsize);
    } else {
        smallerY = start.y;
        biggerY = end.y;
        xIndex = std::floor(end.y * gridsize);
    }
    int firstIndexY = std::ceil(smallerY * gridsize);
    for (int counter = 0; firstIndexY + counter < biggerY * gridsize; counter++) {
        edgeBucketsHorizontal[xIndex][firstIndexY + counter].push_back(edgeIndex);
    }
}

// TODO: I think antimeridian case does not work yet -> Fix later
void TransitNodesRouting::findEdgeBuckets() {
    edgeBucketsHorizontal = std::vector<std::vector<std::vector<int>>>(gridsize);
    edgeBucketsVertical = std::vector<std::vector<std::vector<int>>>(gridsize);
    for (int i = 0; i < graph->sources.size(); i++) {
        // constructor of Vec2 creates an equidistant cylindrical projection on a unit interval [0, 1]
        Vec2 startProj = Vec2(graph->nodes[graph->sources[i]]);
        Vec2 endProj = Vec2(graph->nodes[graph->targets[i]]);
        fillBucketsVertical(startProj, endProj, i);
        fillBucketsHorizontal(startProj, endProj, i);
    }
}