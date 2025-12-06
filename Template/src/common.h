#pragma once

#ifndef COMMON_H
#define COMMON_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>

// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees)
{
    return degrees * pi / 180.0;
}

inline double random_double()
{
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
    return min + (max - min) * random_double();
}

inline int random_int(int min, int max)
{
    return int(random_double(min, max + 1));
}

inline int median(std::vector<int> v)
{
    auto s = v.size();
    return v[(s-1)/2];
}

inline int average(std::vector<int> v)
{
    int sum = 0;

    for (auto i : v)
        sum += i;

    return sum / v.size();
}

// Common Headers

#include "interval.h"
#include "ray.h"
#include "Vec3.h"

#endif
