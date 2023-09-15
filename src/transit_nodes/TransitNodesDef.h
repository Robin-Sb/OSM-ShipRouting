#pragma once
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <unordered_map>
#include <vector>
#include "../utils/Utils.h"


enum RelativePosition {
    NEGATIVE,
    POSITIVE,
    UNDEF
};

struct NodeDistance {
    NodeDistance(int _nodeIndex, int _distanceToV) {
        nodeIndex = _nodeIndex;
        distanceToV = _distanceToV;
    }
    int nodeIndex;
    int distanceToV;
};

struct DistanceData {
    DistanceData(int _nodeId, std::unordered_map<int, int> _distanceToV, Vec2 _cell) {
        referenceNodeIndex = _nodeId;
        distanceToV = _distanceToV;
        cell = _cell;
    }
    Vec2 cell;
    int referenceNodeIndex;
    std::unordered_map<int, int> distanceToV;
};

struct BoundaryNodeData {
    BoundaryNodeData() {
        isAtCellBoundary = false;
        localIndex = -1;
        relPos = RelativePosition::UNDEF;
    }
    BoundaryNodeData(bool _isAtCellBoundary, int _localIndex, RelativePosition _relPos) {
        isAtCellBoundary = _isAtCellBoundary;
        localIndex = _localIndex;
        relPos = _relPos;
    }
    std::vector<int> edges;
    bool isAtCellBoundary;
    int localIndex;
    RelativePosition relPos;
};

struct TransitNodesData {
    friend class boost::serialization::access;
    TransitNodesData(std::vector<int> _transitNodes, std::vector<std::vector<int>> _distancesBetweenTransitNodes, std::vector<std::vector<std::vector<int>>> _transitNodesPerCell, std::vector<std::vector<int>> _distancesToLocalTransitNodes, int _gridsize_x, int _gridsize_y) {
        transitNodes = _transitNodes;
        distancesBetweenTransitNodes = _distancesBetweenTransitNodes;
        transitNodesPerCell = _transitNodesPerCell;
        distancesToLocalTransitNodes = _distancesToLocalTransitNodes;
        gridsize_x = _gridsize_x;
        gridsize_y = _gridsize_y;
    }

    TransitNodesData(){};
    std::vector<int> transitNodes;
    std::vector<std::vector<int>> distancesBetweenTransitNodes;
    std::vector<std::vector<std::vector<int>>> transitNodesPerCell;
    std::vector<std::vector<int>> distancesToLocalTransitNodes;
    int gridsize_x;
    int gridsize_y;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & transitNodes;
        ar & distancesBetweenTransitNodes;
        ar & transitNodesPerCell;
        ar & distancesToLocalTransitNodes;
        ar & gridsize_x;
        ar & gridsize_y;
    }
};
