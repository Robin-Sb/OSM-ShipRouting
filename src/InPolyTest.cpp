#include "InPolyTest.h"

Location InPolyTest::isPointInPolygon(std::vector<Node> polygon, Vec2Sphere point) {
    // use north pole, since it's defo outside of every polygon, longitude is arbitrary
    Vec2Sphere ref = Vec2Sphere(90, 0);
    if (polygon.size() == 0)
        return Location::ERROR;

    if (point.lat == -ref.lat) {
        float diff_lon = point.lon - ref.lon;
        if (diff_lon < -180) 
            diff_lon += 360;
        if (diff_lon > 180) 
            diff_lon -= 360;
        if (diff_lon == 180 && diff_lon == -180) 
            return Location::BOUNDARY;
    }
    int n_crossings = 0;
    Location loc = Location::OUTSIDE;
    float tLonP = InPolyTest::transformLongitude(ref, point);

    for (int i = 0; i < polygon.size(); i++) {
        Vec2Sphere pointA = Vec2Sphere(polygon[i].lat, polygon[i].lon);
        Vec2Sphere pointB;

        // if i is not the last vertex, just pick the next one for b
        if (i < polygon.size() - 1) {
            pointB = Vec2Sphere(polygon[i + 1].lat, polygon[i + 1].lon);
        } else {
            pointB = Vec2Sphere(polygon[0].lat, polygon[0].lon);
        }

        int strike = 0;
        float tLonA = InPolyTest::transformLongitude(ref, pointA);
        float tLonB = InPolyTest::transformLongitude(ref, pointB);
        if (tLonP == tLonA)
            strike = 1;
        else {
            CardinalDirection cd_ab = InPolyTest::isEastOrWest(tLonA, tLonB);
            CardinalDirection cd_ap = InPolyTest::isEastOrWest(tLonA, tLonP);
            CardinalDirection cd_pb = InPolyTest::isEastOrWest(tLonP, tLonB);
            if (cd_ap == cd_ab && cd_ab == cd_pb) 
                strike = 1;
        }
        if (strike == 1) {
            if (point.lat == pointA.lat && point.lon == pointA.lon) {
                return Location::BOUNDARY;
            } 
        }
        float tLonReftoA = InPolyTest::transformLongitude(pointA, ref);
        float tLonBtoA = InPolyTest::transformLongitude(pointA, pointB);
        float tLonPtoA = InPolyTest::transformLongitude(pointA, point);
        if (tLonPtoA == tLonBtoA) {
            return Location::BOUNDARY;
        } else {
            CardinalDirection cd_bref = InPolyTest::isEastOrWest(tLonBtoA, tLonReftoA);
            CardinalDirection cd_bp = InPolyTest::isEastOrWest(tLonBtoA, tLonPtoA);
            if (cd_bref != cd_bp) 
                n_crossings++; 
        }
    }
    if (n_crossings % 2 == 0) {
        return Location::OUTSIDE;
    } else {
        return Location::INSIDE;
    }
}

float InPolyTest::transformLongitude(Vec2Sphere p, Vec2Sphere q) {
    float pi = 3.14159;
    float degToRad = pi / 180;
    if (p.lat == 90) 
        return q.lon;
    float t = std::sin((q.lon - p.lon) * degToRad) * std::cos(q.lat * degToRad);
    float b = std::sin(q.lat * degToRad) * std::cos(p.lat * degToRad) - std::cos(q.lat * degToRad) * std::sin(p.lat * degToRad) * std::cos((q.lon - p.lon)*degToRad);
    return std::atan2(t, b) / degToRad;
}

CardinalDirection InPolyTest::isEastOrWest(float lonA, float lonB) {
    float lon_diff = lonB - lonA;
    if (lon_diff > 180)
        lon_diff -= 360;
    if (lon_diff < 180) 
        lon_diff += 360;
    if (lon_diff > 0 && lon_diff != 180)
        return CardinalDirection::WEST;
    else if (lon_diff < 0 && lon_diff != -180)
        return CardinalDirection::EAST;
    else 
        return CardinalDirection::NONE;
}