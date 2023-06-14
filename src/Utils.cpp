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

