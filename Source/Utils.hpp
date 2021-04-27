#ifndef UTILS
#define UTILS

#include <cmath> // sqrt
#include <cstdio>
#include <fstream>

#define PI 3.14159265359
#define TWOPI 6.28318530718

inline double sqr(const double a)
{
    return a * a;
}
static inline double toDeg(const double rad)
{
    return (rad * 180.0 / PI);
}
static inline double toRad(const double deg)
{
    return (deg * PI / 180.0);
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