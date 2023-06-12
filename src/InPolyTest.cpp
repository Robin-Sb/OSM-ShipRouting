#include "InPolyTest.h"

BoundingPolygon::BoundingPolygon(float _latMin, float _latMax, float _lonMin, float _lonMax) {
    latMin = _latMin;
    latMax = _latMax;
    lonMin = _lonMin;
    lonMax = _lonMax;
}

bool BoundingPolygon::isInside(Vec2Sphere point) {
    float newLon = point.lon;
    // handle special antimeridian case with new transformed coordinates
    if (lonMax > 180) {
        if (point.lon < 0) {
            newLon = 180 + (180 + point.lon);
        }
    }
    if (point.lat < latMin || point.lat > latMax) 
        return false;
    if (newLon < lonMin || newLon > lonMax)
        return false;
    return true;
}

InPolyTest::InPolyTest(std::vector<SingleCoast> _coastlines) {
    coastlines = _coastlines;
    for (int i = 0; i < coastlines.size(); i++) {
        BoundingPolygon bp = BoundingPolygon(coastlines[i].path[0].lat, coastlines[i].path[0].lat, coastlines[i].path[0].lon, coastlines[i].path[0].lon);
        
        bool crossesAntiMeridian = false;
        for (int j = 1; j < coastlines[i].path.size(); j++) {
            bp.latMax = std::max(coastlines[i].path[j].lat, bp.latMax);
            bp.latMin = std::min(coastlines[i].path[j].lat, bp.latMin);
            bp.lonMin = std::min(coastlines[i].path[j].lon, bp.lonMin);
            bp.lonMax = std::max(coastlines[i].path[j].lon, bp.lonMax);
            Node start = coastlines[i].path[j];
            Node end;
            if (j < coastlines[i].path.size() - 1)
                end = coastlines[i].path[j + 1];
            else
                end = coastlines[i].path[0];
            // very hacky test whether a line crosses the antimeridian
            if ((start.lon > 170 && end.lon < -170) || (start.lon < -170 && end.lon > 170))
                crossesAntiMeridian = true;
        }
        //std::cout << bp.latMin << std::endl;
        // if polygon crosses antimeridian
        // kind of hacky but in this case we transform the negative longitude to positive (range [0, 360])
        if (crossesAntiMeridian) {
            float oldMin = bp.lonMin;
            bp.lonMin = bp.lonMax;
            bp.lonMax = 180 + (180 + oldMin);
        }

        //std::cout << bp.latMin << std::endl;

        // check if south pole in polygon -> special case
        Vec2Sphere southpole = Vec2Sphere(-85.0, 45);
        Location polyTestResult = isPointInPolygon(coastlines[i].path, southpole);
        if (polyTestResult == Location::INSIDE) {
            bp.latMin = -90;
            bp.lonMax = 180;
            bp.lonMin = -180;
        }
        bps.push_back(bp);
    }
}

bool InPolyTest::performPointInPolyTest(Vec2Sphere point) {
    bool isOutside = true;
    for (int i = 0; i < coastlines.size(); i++) {
        if (bps[i].isInside(point)) {
            Location polyTestResult = isPointInPolygon(coastlines[i].path, point);
            if (polyTestResult == Location::INSIDE || polyTestResult == Location::BOUNDARY) {
                isOutside = false;
                break;
            }
        }
    }
    return !isOutside;
}

Location InPolyTest::isPointInPolygon(std::vector<Node> &polygon, Vec2Sphere point) {
    // use north pole, since it's defo outside of every polygon, longitude is arbitrary
    Vec2Sphere ref = Vec2Sphere(26, -47);
    if (polygon.size() == 0)
        return Location::ERROR;

    if (point.lat == -ref.lat) {
        float diff_lon = point.lon - ref.lon;
        if (diff_lon < -180) 
            diff_lon += 360;
        if (diff_lon > 180) 
            diff_lon -= 360;
        if (diff_lon == 180 || diff_lon == -180) 
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
    }
    if (n_crossings % 2 == 0) {
        return Location::OUTSIDE;
    } else {
        return Location::INSIDE;
    }
}

float InPolyTest::transformLongitude(Vec2Sphere p, Vec2Sphere q) {
    float pi = 3.14159;
    float degToRad = pi / 180.0;
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
    if (lon_diff < -180) 
        lon_diff += 360;
    if (lon_diff > 0 && lon_diff != 180)
        return CardinalDirection::WEST;
    else if (lon_diff < 0 && lon_diff != -180)
        return CardinalDirection::EAST;
    else 
        return CardinalDirection::NONE;
}