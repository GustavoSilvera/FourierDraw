#ifndef VECT
#define VECT

#include "Utils.hpp"
#include <array>
#include <cmath>

/// TODO: use template magic
#define N 2
class Vec2D
{
  public:
    //////////// :CONSTRUCTORS: //////////////
    Vec2D() : Vec2D(0.0)
    {
    } // delegate constructor, default value 0

    Vec2D(const double x, const double y)
    {
        Data[0] = x;
        Data[1] = y;
    }

    Vec2D(const double init)
    {
        for (size_t i = 0; i < N; i++)
        {
            // allocate data
            Data[i] = init;
        }
    }

    Vec2D(const std::array<double, N> &Copy) // duplicate vector
    {
        for (size_t i = 0; i < N; i++)
        {
            // Copy data over
            Data[i] = Copy[i];
        }
    }

    double SizeSqr() const
    {
        double sum = 0;
        for (size_t i = 0; i < N; i++)
        {
            sum += sqr(Data[i]);
        }
        return sum;
    }

    double Size() const
    {
        return std::sqrt(SizeSqr());
    }

    Vec2D Norm() const
    {
        // divide by magnitude
        return Vec2D(Data) / Size();
    }

    Vec2D rotate(const double angle) const
    {
        /// TODO: make one that operates on vector itself
        const double rX = cos(angle) * Data[0] - sin(angle) * Data[1];
        const double rY = sin(angle) * Data[0] + cos(angle) * Data[1];
        return Vec2D(rX, rY);
    }

    //////////// :CREATION: //////////////
    Vec2D operator+(const Vec2D &Other) const
    {
        Vec2D Ret(Data);
        for (size_t i = 0; i < N; i++)
        {
            Ret.Data[i] += Other.Data[i];
        }
        return Ret;
    }

    Vec2D operator-(const Vec2D &Other) const
    {
        Vec2D Ret(Data);
        for (size_t i = 0; i < N; i++)
        {
            Ret.Data[i] -= Other.Data[i];
        }
        return Ret;
    }

    Vec2D operator/(const double Denom) const
    {
        Vec2D Ret(Data);
        for (size_t i = 0; i < N; i++)
        {
            Ret.Data[i] /= Denom;
        }
        return Ret;
    }

    /// TODO: add template <typename T>
    Vec2D operator*(const double Scale) const
    {
        Vec2D Ret(Data);
        for (size_t i = 0; i < N; i++)
        {
            Ret.Data[i] *= Scale;
        }
        return Ret;
    }

    //////////// :ASSIGNMENT: //////////////
    void operator+=(const Vec2D &Other)
    {
        for (size_t i = 0; i < N; i++)
        {
            Data[i] += Other.Data[i];
        }
    }

    void operator-=(const Vec2D &Other)
    {
        for (size_t i = 0; i < N; i++)
        {
            Data[i] -= Other.Data[i];
        }
    }

    void operator/=(const double Denom)
    {
        for (size_t i = 0; i < N; i++)
        {
            Data[i] /= Denom;
        }
    }

    void operator*=(const double Scale)
    {
        for (size_t i = 0; i < N; i++)
        {
            Data[i] *= Scale;
        }
    }

    void operator=(const Vec2D &Other)
    {
        for (size_t i = 0; i < N; i++)
        {
            // ensures elements are present
            Data[i] = Other[i];
        }
    }

    //////////// :GETTERS: //////////////
    double operator[](const size_t i) const
    {
        return Data[i];
    }

    // Easiest if Data is public
    std::array<double, N> Data; /// TODO: template the size
};

#endif