#ifndef IMAGE_H
#define IMAGE_H

#include "Utils.hpp"
#include "Vec.hpp"
#include <cassert>
#include <cmath> // pow
#include <fstream>
#include <iostream>
#include <vector>

class Colour
{
  public:
    Colour(int r, int g, int b)
    {
        R = r;
        G = g;
        B = b;
    }
    Colour()
    {
        Colour(0, 0, 0);
    }
    double R;
    double G;
    double B;

    Colour Norm0_1() const
    {
        return Colour(R / 255.0, G / 255.0, B / 255.0);
    }

    Colour Norm0_255() const
    {
        return Colour(R * 255.0, G * 255.0, B * 255.0);
    }
};

class Image
{
  public:
    Image() = default;

    Image(const size_t TiD)
    {
        if (Params.Simulator.Render)
        {
            // Initialize all the data
            for (size_t i = 0; i < Params.Image.WindowX; i++)
            {
                std::vector<Colour> Row;
                for (size_t j = 0; j < Params.Image.WindowY; j++)
                {
                    Row.push_back(Colour(0, 0, 0)); // all black
                }
                Data.push_back(Row);
            }
            assert(Data.size() == W && Data[0].size == H);
        }
        ThreadID = TiD;
        NumExported = 0;
    }

    std::vector<std::vector<Colour>> Data;
    size_t NumExported, ThreadID;
    void SetPixel(const size_t X, const size_t Y, const Colour C)
    {
        /// TODO: do we need to check bounds always? even with EdgeWrap?
        const bool WithinWidth = (X < Params.Image.WindowX);
        const bool WithinHeight = (Y < Params.Image.WindowY);
        if (WithinWidth && WithinHeight) // draw boid within bound (triangle)
        {
            Data[X][Y] = C;
        }
    }
    void SetPixel(const Vec2D Pos, const Colour C)
    {
        SetPixel(Pos[0], Pos[1], C);
    }

    void SetPixelW(const double X, const double Y, const Colour C) // sets pixel with wrapping
    {
        const double MaxW = Params.Image.WindowX - 1;
        const double MaxH = Params.Image.WindowY - 1;
        double ClampedX = X;
        if (ClampedX < 0)
        {
            ClampedX += MaxW;
        }
        else if (ClampedX > MaxW)
        {
            ClampedX -= MaxW;
        }
        /// same for y's
        double ClampedY = Y;
        if (ClampedY < 0)
        {
            ClampedY += MaxH;
        }
        else if (ClampedY > MaxH)
        {
            ClampedY -= MaxH;
        }
        Data[ClampedX][ClampedY] = C;
    }

    void Blank()
    {
        // #pragma omp parallel for
        const size_t BorderSize = 0;
        for (int i = 0; i < Params.Image.WindowX; i++)
        {
            for (int j = 0; j < Params.Image.WindowY; j++)
            {
                if (i < BorderSize || i > Params.Image.WindowX - BorderSize || j < BorderSize ||
                    j > Params.Image.WindowY - BorderSize)
                {
                    SetPixel(i, j, Colour(255, 255, 255));
                }
                else
                {
                    SetPixel(i, j, Colour(0, 0, 0));
                }
            }
        }
    }

    void DrawSolidCircle(const Vec2D &Center, const size_t Radius, const Colour &C)
    {
        const double X = Center[0];
        const double Y = Center[1];
        for (double pX = X - Radius; pX < X + Radius; pX++)
        {
            for (double pY = Y - Radius; pY < Y + Radius; pY++)
            {
                /// TODO: provide functionality for custom-sized radii
                if (sqr(pX - X) + sqr(pY - Y) < sqr(Radius) && sqr(pX - X) + sqr(pY - Y) > sqr(Radius - 1))
                {
                    SetPixel(pX, pY, C);
                }
            }
        }
    }

    void SetOctants(const double X, const double x, const double Y, const double y, const Colour &C)
    {
        SetPixel(X + x, Y + y, C);
        SetPixel(X - x, Y + y, C);
        SetPixel(X + x, Y - y, C);
        SetPixel(X - x, Y - y, C);
        if (x != y)
        {
            SetPixel(X + y, Y + x, C);
            SetPixel(X - y, Y + x, C);
            SetPixel(X + y, Y - x, C);
            SetPixel(X - y, Y - x, C);
        }
    }

    void DrawStrokedCircle(const Vec2D &Center, const size_t Radius, const Colour &C)
    {
        // Faster Stroked circle, uses Mid-point circle drawing algorithm
        /// NOTE: this is slow
        const double X = Center[0];
        const double Y = Center[1];

        // Initialising the value of P
        int P = 1 - Radius;
        int x = Radius;
        int y = 0;
        while (x > y)
        {
            y++;
            if (P <= 0) // within perimiter
            {
                P = P + 2 * y + 1;
            }
            else // outside perimiter
            {
                x--;
                P = P + 2 * y - 2 * x + 1;
            }
            if (x < y)
                break;
            SetOctants(X, x, Y, y, C);
        }
    }

    void DrawStrokedCircleNaive(const Vec2D &Center, const size_t Radius, const Colour &C)
    {
        const double X = Center[0];
        const double Y = Center[1];

        for (double pX = X - Radius; pX < X + Radius; pX++)
        {
            for (double pY = Y - Radius; pY < Y + Radius; pY++)
            {
                /// TODO: provide functionality for custom-sized radii
                if (sqr(pX - X) + sqr(pY - Y) < sqr(Radius) && sqr(pX - X) + sqr(pY - Y) > sqr(Radius - 1))
                {
                    SetPixel(pX, pY, C);
                }
            }
        }
    }

    void DrawLine(const Vec2D &A, const Vec2D &B, const Colour &C)
    {
        Vec2D Direction = B - A;
        const double Magnitude = Direction.Size();
        Direction /= Magnitude; // normalize it
        /// TODO: add more granularity for lines
        for (size_t i = 0; i < Magnitude; i++)
        {
            /// TODO: verify this works
            const Vec2D Pixel = A + Direction * i;
            SetPixel(Pixel, C);
        }
    }

    void ExportPPMImage()
    {
        const size_t NumLeading0s = 4; // max 9999 frames
        const size_t MaxFrames = std::pow(10, NumLeading0s);
        const size_t ImgID = NumExported * Params.Simulator.NumThreads + ThreadID;
        if (ImgID > MaxFrames)
        {
            std::cout << "Cannot export more than " << MaxFrames << " frames! " << std::endl;
            return;
        }
        std::string Path = "Out/";
        std::string NumStr = std::to_string(ImgID); // which frame this is
        std::string Filename = Path + std::string(NumLeading0s - NumStr.length(), '0') + NumStr + ".ppm";

        // Begin writing output stream
        std::ofstream Img(Filename, std::ios_base::out | std::ios_base::binary);
        Img << "P6" << std::endl
            << Params.Image.WindowX << " " << Params.Image.WindowY << std::endl
            << "255" << std::endl; // write ppm header
        for (size_t j = 0; j < Params.Image.WindowY; ++j)
        {
            for (size_t i = 0; i < Params.Image.WindowX; ++i)
            {
                const Colour RGB = Data[i][j];
                Img << char(RGB.R) << char(RGB.G) << char(RGB.B);
            }
        }
        Img.close();
        NumExported++; // exported a new file
        if (ThreadID == 0)
        {
            std::cout << "(t0) Wrote image file: " << Filename << "\r" << std::flush; // carriage return, no newline
        }
    }
};

#endif