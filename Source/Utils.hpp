#ifndef UTILS
#define UTILS

#include <cmath> // sqrt
#include <cstdio>
#include <fstream>

inline double sqr(const double a)
{
    return a * a;
}
static inline double toDeg(const double rad)
{
    return (rad * 180 / M_PI);
}
static inline double toRad(const double deg)
{
    return (deg * M_PI / 180);
}
static inline double clamp(const double min, const double max, const double i)
{
    if (i > min)
    {
        if (i < max)
        {
            return i;
        }
        return max;
    }
    return min;
}

#endif