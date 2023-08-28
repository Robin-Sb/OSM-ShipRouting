#pragma once
#include <vector>
#include "math.h"
#include <memory>
#include <algorithm>
#include <numeric>


const float pi = 3.14159265359;

template <typename T, typename Compare>
std::vector<std::size_t> sort_permutation(
    const std::vector<T>& vec,
    Compare compare)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
        [&](std::size_t i, std::size_t j){ return compare(vec[i], vec[j]); });
    return p;
}

template <typename T>
std::vector<T> apply_permutation(
    const std::vector<T>& vec,
    const std::vector<std::size_t>& p)
{
    std::vector<T> sorted_vec(vec.size());
    std::transform(p.begin(), p.end(), sorted_vec.begin(),
        [&](std::size_t i){ return vec[i]; });
    return sorted_vec;
}

template <typename T>
void apply_permutation_in_place(
    std::vector<T>& vec,
    const std::vector<std::size_t>& p)
{
    std::vector<bool> done(vec.size());
    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        if (done[i])
        {
            continue;
        }
        done[i] = true;
        std::size_t prev_j = i;
        std::size_t j = p[i];
        while (i != j)
        {
            std::swap(vec[prev_j], vec[j]);
            done[j] = true;
            prev_j = j;
            j = p[j];
        }
    }
}


class Vec2Sphere {
    public:
        Vec2Sphere() {};
        Vec2Sphere(float lat, float lon);
        
        float lat;
        float lon;
        static float dist(Vec2Sphere ref, Vec2Sphere comp);
};


class Vec2 {
    public: 
        Vec2() {};
        Vec2(float _x, float _y);
        Vec2(Vec2Sphere sphereBase);

        float x;
        float y;
        static float projectX(float lon);
        static float projectY(float lat);
    private:
        static float degToRad(float angleDeg);
};

class UtilFunctions {
    public: 
        static int getCellX(Vec2Sphere &v, int gridX);
        static int getCellY(Vec2Sphere &v, int gridY);

        static bool sameCell(Vec2Sphere &v1, Vec2Sphere &v2, int gridsize);
};