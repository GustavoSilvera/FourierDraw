#ifndef ARROW
#define ARROW

#include "Utils.hpp"
#include "Vec.hpp"

class Arrow
{
  public:
    Arrow(const double L, const double V, const double T, const Vec2D &P)
    {
        Length = L;
        Velocity = V;
        Theta = T;
        Position = P;
    }
    Arrow(const double L, const double V, const double T)
    {
        Length = L;
        Velocity = V;
        Theta = T;
    }
    Arrow() : Arrow(1, 0, M_PI / 2.0)
    {
    }
    double Length = 0;   // meters
    double Velocity = 0; // radians/sec
    double Theta = 0;    // radians
    Vec2D Position;      // (x meters, y meters, 0)
    Vec2D Tip() const    // gets the tip end of the arrow in rectangular coords
    {
        // converts angular(polar) to rectangular
        Vec2D Offset(Length * std::cos(Theta), Length * std::sin(Theta));
        return Position + Offset; // adds arrowhead to initial 2D rectangular position
    }
};

#endif