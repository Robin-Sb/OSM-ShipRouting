#include "Utils.h"

Vec2Sphere::Vec2Sphere(float _lat, float _lon) {
    lat = _lat;
    lon = _lon;
}

float Vec2Sphere::dist(Vec2Sphere ref, Vec2Sphere comp) {
    // earth radius in m
    int radius = 6371000;
    float pi = 3.14159265359;
    float deg_to_rad = pi / 180;
    float lat1_rad = ref.lat * deg_to_rad;
    float lat2_rad = comp.lat * deg_to_rad;
    float diff_lat = (comp.lat - ref.lat) * deg_to_rad;
    float diff_lon = (comp.lon - ref.lon) * deg_to_rad;

    float a = std::sin(diff_lat/2) * std::sin(diff_lat/2) + std::cos(lat1_rad) * std::cos(lat2_rad) * std::sin(diff_lon/2) * std::sin(diff_lon/2);
    float c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    return radius * c;
}

Vec2::Vec2(float _x, float _y) {
    x = _x;
    y = _y;
}

Vec2::Vec2(Vec2Sphere sphereBase) {
    // float pi = 3.14159265359;
    // float deg_to_rad = pi / 180;
    // // define on a unit interval
    // float lon = sphereBase.lon * deg_to_rad;
    // float lat = sphereBase.lat * deg_to_rad;
    // mercator
    //x = ((lon - 0) + pi) / (2 * pi);
    //y = (std::asinh(std::tan(lat)) + (pi / 2)) / pi;
    // miller
    //float max_y = 5.0/4.0 * std::asinh(std::tan((4.0/5.0) * (pi/2)));
    //y = (5.0/4.0 * std::asinh(std::tan(4.0/5.0 * lat)) + max_y) / (max_y * 2);
    // plate carree
    x = UtilFunctions::projectX(sphereBase.lon);
    y = UtilFunctions::projectY(sphereBase.lat);
    //y = ((lat - 0) + (pi / 2)) / pi;
    //y = std::asinh(std::tan(sphereBase.lat * deg_to_rad));
}


float UtilFunctions::degToRad(float angleDeg) {
    return angleDeg * (pi / 180.0);
}

float UtilFunctions::radToDeg(float angleRad) {
    return angleRad * (180.0 / pi);
}

float UtilFunctions::unprojectX(float x) {
    return UtilFunctions::radToDeg((x * 2 * pi) - pi);
}

float UtilFunctions::unprojectY(float y) {
    return UtilFunctions::radToDeg((y * 2 * pi) - (pi / 2));
}

float UtilFunctions::projectX(float lon) {
    lon = UtilFunctions::degToRad(lon);
    return ((lon - 0) * std::cos(0) + pi) / (2 * pi);
} 

float UtilFunctions::projectY(float lat) {
    lat = UtilFunctions::degToRad(lat);
    return ((lat - 0) + (pi / 2)) / (2 * pi);
}

bool UtilFunctions::sameCell(Vec2Sphere &v1, Vec2Sphere &v2, int gridsize) {
    if (getCellX(v1, gridsize) == getCellX(v2, gridsize) &&
        getCellY(v1, gridsize) == getCellY(v2, gridsize)) 
        return true;
    return false;
}

int UtilFunctions::getCellX(Vec2Sphere &v, int gridX) {
    float x = UtilFunctions::projectX(v.lon);
    return std::floor(x * gridX);
}

int UtilFunctions::getCellY(Vec2Sphere &v, int gridY) {
    float y = UtilFunctions::projectY(v.lat);
    return std::floor(y * gridY);
}
