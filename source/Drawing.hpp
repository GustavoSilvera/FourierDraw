#ifndef DRAWING
#define DRAWING

#include "Arrow.hpp"
#include "Fourier.hpp"
#include "Image.hpp"
#include "Utils.hpp"
#include "Vec.hpp"
#include <vector>

class Drawing
{
  public:
    Drawing() = default;
    Drawing(const size_t NA, const double DT, const bool PD, const Vec2D &WindowSize)
    {
        NumArrows = NA;
        DeltaTime = DT;
        PenDown = PD;
        I = I.Init(WindowSize);
        InitialPosition = Vec2D(0, 0); // WindowSize / 2;
        // first arrow
        AddArrow(Arrow(1, 1, 0, InitialPosition));
        // initialize all the arrows from the fourier transform
        FourierInit();
    }
    size_t NumArrows;
    double DeltaTime;
    bool PenDown;
    Image I;

    std::vector<Arrow> Train; // Train of arrows
    std::vector<Vec2D> Path;  // path drawn by final arrow
    Vec2D InitialPosition;

    void AddArrow(const Arrow &A)
    {
        Train.push_back(A);
    }

    void FourierInit()
    {
        /// TODO: don't do this
        std::vector<Complex> Coordinates = {
            Complex{-2.83, -2.1},  Complex{-3.11, -2.2},  Complex{-3.38, -2.2},  Complex{-3.61, -2.09},
            Complex{-3.79, -1.95}, Complex{-3.95, -1.77}, Complex{-4.1, -1.53},  Complex{-4.14, -1.24},
            Complex{-4.14, -1.06}, Complex{-4.02, -0.92}, Complex{-3.9, -0.87},  Complex{-3.63, -0.86},
            Complex{-3.55, -0.94}, Complex{-3.38, -1.06}, Complex{-3.35, -1.1},  Complex{-3.38, -1.01},
            Complex{-3.42, -0.88}, Complex{-3.42, -0.76}, Complex{-3.43, -0.56}, Complex{-3.5, -0.43},
            Complex{-3.58, -0.31}, Complex{-3.69, -0.21}, Complex{-3.82, -0.15}, Complex{-3.82, -0.35},
            Complex{-3.73, -0.43}, Complex{-3.62, -0.5},  Complex{-3.46, -0.53}, Complex{-3.2, -0.67},
            Complex{-3.07, -0.75}, Complex{-2.97, -0.83}, Complex{-2.84, -0.95}, Complex{-2.67, -1.07},
            Complex{-2.61, -1.14}, Complex{-2.57, -1.19}, Complex{-2.44, -0.96}, Complex{-2.33, -0.89},
            Complex{-2.18, -0.84}, Complex{-1.97, -0.86}, Complex{-1.86, -0.89}, Complex{-1.73, -0.94},
            Complex{-1.64, -1},    Complex{-1.54, -1.07}, Complex{-1.38, -1.07}, Complex{-1.25, -1.13},
            Complex{-1.13, -1.21}, Complex{-1.02, -1.3},  Complex{-0.96, -1.42}, Complex{-0.95, -1.5},
            Complex{-1.01, -1.52}, Complex{-1.06, -1.46}, Complex{-1.11, -1.39}, Complex{-1.21, -1.31},
            Complex{-1.34, -1.26}, Complex{-1.42, -1.22}, Complex{-1.44, -1.24}, Complex{-1.38, -1.35},
            Complex{-1.34, -1.51}, Complex{-1.33, -1.68}, Complex{-1.32, -1.79}, Complex{-1.33, -1.93},
            Complex{-1.39, -2.07}, Complex{-1.5, -2.18},  Complex{-1.68, -2.21}, Complex{-1.79, -2.21},
            Complex{-1.94, -2.17}, Complex{-2.11, -2.09}, Complex{-2.18, -2.03}, Complex{-2.24, -1.98},
            Complex{-2.32, -1.91}, Complex{-2.39, -1.79}, Complex{-2.43, -1.65}, Complex{-2.5, -1.53},
            Complex{-2.53, -1.47}, Complex{-2.59, -1.36}, Complex{-2.69, -1.26}, Complex{-2.78, -1.16},
            Complex{-2.83, -1.11}, Complex{-2.93, -1.05}, Complex{-2.99, -1.01}, Complex{-3.06, -0.96},
            Complex{-3.09, -0.94}, Complex{-3.46, -0.78}, Complex{-3.57, -0.75}, Complex{-3.66, -0.69},
            Complex{-3.76, -0.61}, Complex{-3.9, -0.55},  Complex{-4.01, -0.48}, Complex{-4.06, -0.43},
            Complex{-4.1, -0.31},  Complex{-4.1, -0.23},  Complex{-4.1, -0.12},  Complex{-4.09, -0.1},
            Complex{-4, -0.03},    Complex{-3.91, 0.01},  Complex{-3.81, 0.04},  Complex{-3.71, 0.04},
            Complex{-3.57, -0.01}, Complex{-3.53, -0.07}, Complex{-3.4, -0.15},  Complex{-3.35, -0.21},
            Complex{-3.31, -0.29}, Complex{-3.31, -0.39}, Complex{-3.27, -0.52}, Complex{-3.25, -0.63},
            Complex{-3.16, -0.76}, Complex{-3.16, -0.83}, Complex{-3.16, -0.87}, Complex{-3.14, -0.97},
            Complex{-2.92, -1.71}, Complex{-2.94, -1.81}, Complex{-3.02, -1.82}, Complex{-3.06, -1.82},
            Complex{-3.12, -1.82}, Complex{-3.15, -1.71}, Complex{-3.26, -1.54}, Complex{-3.29, -1.47},
            Complex{-3.38, -1.37}, Complex{-3.52, -1.23}, Complex{-3.67, -1.1},  Complex{-3.78, -1.07},
            Complex{-3.81, -1.08}, Complex{-3.82, -1.27}, Complex{-3.81, -1.5},  Complex{-3.73, -1.67},
            Complex{-3.6, -1.83},  Complex{-3.5, -1.94},  Complex{-3.26, -2},    Complex{-3.14, -1.99},
            Complex{-2.99, -1.93}, Complex{-2.94, -1.91}, Complex{-2.81, -1.87}, Complex{-2.7, -1.95},
            Complex{-2.7, -2.02},  Complex{-2.83, -2.11}, Complex{-2.97, -2.19}, Complex{-3.24, -2.19},
            Complex{-3.49, -2.13}, Complex{-3.7, -1.99},  Complex{-3.86, -1.85}, Complex{-4.04, -1.65},
            Complex{-4.12, -1.38}, Complex{-4.13, -1.04}, Complex{-3.94, -0.92}, Complex{-3.82, -0.87},
            Complex{-3.6, -0.87},  Complex{-3.44, -0.97}, Complex{-3.33, -1.09}, Complex{-3.42, -0.67},
            Complex{-3.55, -0.37}, Complex{-3.62, -0.19}, Complex{-3.78, -0.17}, Complex{-3.91, -0.19},
            Complex{-3.88, -0.3},  Complex{-3.79, -0.39}, Complex{-3.66, -0.48}, Complex{-3.58, -0.52},
            Complex{-3.46, -0.55}, Complex{-3.35, -0.63}, Complex{-3.05, -0.75}, Complex{-2.9, -0.87},
            Complex{-2.77, -0.99}, Complex{-2.66, -1.09}, Complex{-2.61, -1.18}, Complex{-2.56, -1.08},
            Complex{-2.51, -1.03}, Complex{-2.43, -0.95}, Complex{-2.27, -0.87}, Complex{-2.06, -0.84},
            Complex{-1.9, -0.84},  Complex{-1.76, -0.9},  Complex{-1.66, -0.99}, Complex{-1.52, -1.03},
            Complex{-1.46, -1.07}, Complex{-1.34, -1.07}, Complex{-1.21, -1.11}, Complex{-1.12, -1.17},
            Complex{-1.02, -1.3},  Complex{-0.94, -1.42}, Complex{-0.95, -1.56}, Complex{-0.17, -2.14},
            Complex{-0.03, -2.15}, Complex{0.09, -2.15},  Complex{0.16, -2.11},  Complex{0.23, -2.12},
            Complex{0.29, -2.19},  Complex{0.4, -2.21},   Complex{0.44, -2.23},  Complex{0.5, -2.23},
            Complex{0.58, -2.23},  Complex{0.64, -2.21},  Complex{0.7, -2.18},   Complex{0.79, -2.1},
            Complex{0.86, -1.99},  Complex{0.86, -1.87},  Complex{0.86, -1.71},  Complex{0.86, -1.59},
            Complex{0.78, -1.47},  Complex{0.78, -1.35},  Complex{0.74, -1.3},   Complex{0.7, -1.2},
            Complex{0.86, -1.2},   Complex{1.01, -1.28},  Complex{1.1, -1.39},   Complex{1.18, -1.54},
            Complex{1.24, -1.76},  Complex{1.25, -1.87},  Complex{1.3, -1.96},   Complex{1.32, -2.08},
            Complex{1.38, -2.24},  Complex{1.5, -2.47},   Complex{1.58, -2.65},  Complex{1.76, -2.86},
            Complex{1.94, -2.96},  Complex{2.17, -2.97},  Complex{2.27, -2.88},  Complex{2.28, -2.7},
            Complex{2.26, -2.55},  Complex{2.26, -2.43},  Complex{2.22, -2.35},  Complex{2.17, -2.28},
            Complex{2.1, -2.19},   Complex{2.02, -2.07},  Complex{1.89, -1.95},  Complex{1.76, -1.83},
            Complex{1.67, -1.75},  Complex{1.58, -1.7},   Complex{1.48, -1.65},  Complex{1.54, -1.86},
            Complex{1.55, -2.08},  Complex{1.62, -2.23},  Complex{1.68, -2.39},  Complex{1.74, -2.48},
            Complex{1.79, -2.57},  Complex{1.85, -2.64},  Complex{1.89, -2.69},  Complex{1.98, -2.73},
            Complex{2.02, -2.71},  Complex{2.04, -2.59},  Complex{2.02, -2.48},  Complex{1.99, -2.4},
            Complex{1.94, -2.34},  Complex{1.86, -2.23},  Complex{1.79, -2.15},  Complex{1.7, -2.04},
            Complex{1.62, -1.96},  Complex{1.55, -1.88},  Complex{1.54, -1.83},  Complex{1.47, -1.7},
            Complex{1.45, -1.56},  Complex{1.45, -1.43},  Complex{1.45, -1.34},  Complex{1.43, -1.23},
            Complex{1.48, -1.15},  Complex{1.54, -1.09},  Complex{1.62, -1.05},  Complex{1.74, -1.05},
            Complex{1.88, -1.12},  Complex{1.96, -1.22},  Complex{2.01, -1.27},  Complex{2.09, -1.37},
            Complex{2.2, -1.57},   Complex{2.25, -1.69},  Complex{2.32, -1.81},  Complex{2.44, -1.9},
            Complex{2.58, -2},     Complex{2.65, -2.06},  Complex{2.83, -2.15},  Complex{2.96, -2.16},
            Complex{3.12, -2.16},  Complex{3.25, -2.16},  Complex{3.38, -2.16},  Complex{3.46, -2.12},
            Complex{3.52, -2.07},  Complex{3.55, -2.03},  Complex{3.53, -1.94},  Complex{3.48, -1.87},
            Complex{3.38, -1.91},  Complex{3.34, -1.94},  Complex{3.22, -1.95},  Complex{3.1, -1.96},
            Complex{3.02, -1.96},  Complex{2.88, -1.96},  Complex{2.74, -1.86},  Complex{2.65, -1.79},
            Complex{2.56, -1.68},  Complex{2.5, -1.59},   Complex{2.45, -1.47},  Complex{2.43, -1.28},
            Complex{2.43, -1.2},   Complex{2.47, -1.11},  Complex{2.56, -1.09},  Complex{2.65, -1.09},
            Complex{2.74, -1.17},  Complex{2.8, -1.29},   Complex{2.86, -1.39},  Complex{2.9, -1.45},
            Complex{2.93, -1.52},  Complex{2.98, -1.62},  Complex{3.01, -1.7},   Complex{3.03, -1.78},
            Complex{3.05, -1.84},  Complex{3.12, -1.83},  Complex{3.25, -1.82},  Complex{3.34, -1.82},
            Complex{3.3, -1.73},   Complex{3.27, -1.63},  Complex{3.26, -1.48},  Complex{3.23, -1.35},
            Complex{3.22, -1.26},  Complex{3.21, -1.21},  Complex{3.22, -1.17},  Complex{3.25, -1.13},
            Complex{3.3, -1.12},   Complex{3.36, -1.12},  Complex{3.44, -1.16},  Complex{3.51, -1.25},
            Complex{3.56, -1.31},  Complex{3.62, -1.42},  Complex{3.67, -1.48},  Complex{3.71, -1.6},
            Complex{3.74, -1.71},  Complex{3.78, -1.84},  Complex{3.8, -1.92},   Complex{3.83, -1.99},
            Complex{3.87, -2.09},  Complex{3.88, -2.12},  Complex{3.97, -2.12},  Complex{4.04, -2.15},
            Complex{4.06, -2.31},  Complex{3.98, -2.39},  Complex{3.97, -2.51},  Complex{4.01, -2.59},
            Complex{4.1, -2.64},   Complex{4.16, -2.66},  Complex{4.22, -2.65},  Complex{4.26, -2.62},
            Complex{4.29, -2.55},  Complex{4.29, -2.47},  Complex{4.29, -2.44},  Complex{4.24, -2.39},
            Complex{4.2, -2.35},   Complex{4.14, -2.32},  Complex{4.08, -2.13},  Complex{4.18, -2.13},
            Complex{4.18, -2.03},  Complex{4.14, -1.96},  Complex{4.1, -1.89},   Complex{4.06, -1.76},
            Complex{4.05, -1.65},  Complex{3.98, -1.47},  Complex{3.96, -1.35},  Complex{3.94, -1.23},
            Complex{3.94, -1.16},  Complex{3.94, -1.13},  Complex{4.02, -1.1},   Complex{4.1, -1.11},
            Complex{4.19, -1.19},  Complex{4.28, -1.27},  Complex{4.34, -1.39},  Complex{4.36, -1.55},
            Complex{4.39, -1.63},  Complex{4.48, -1.77},  Complex{4.54, -1.84},  Complex{4.62, -1.91},
            Complex{4.7, -1.99},   Complex{4.78, -2.04},  Complex{4.86, -2.11},  Complex{4.94, -2.15},
            Complex{5.07, -2.16},  Complex{5.21, -2.17},  Complex{5.27, -2.17},  Complex{5.33, -2.17},
            Complex{5.64, -3.01},  Complex{5.9, -3},      Complex{5.94, -2.9},   Complex{5.88, -2.79},
            Complex{5.82, -2.63},  Complex{5.76, -2.5},   Complex{5.7, -2.36},   Complex{5.69, -2.18},
            Complex{5.58, -1.99},  Complex{5.52, -1.72},  Complex{5.48, -1.51},  Complex{5.46, -1.35},
            Complex{5.47, -1.16},  Complex{5.51, -1.11},  Complex{5.62, -1.11},  Complex{5.74, -1.19},
            Complex{5.82, -1.29},  Complex{5.88, -1.43},  Complex{5.93, -1.49},  Complex{5.98, -1.58},
            Complex{6.02, -1.55},  Complex{6.06, -1.5},   Complex{6.04, -1.4},   Complex{5.98, -1.32},
            Complex{5.91, -1.24},  Complex{5.86, -1.16},  Complex{5.8, -1.07},   Complex{5.75, -1.03},
            Complex{5.69, -0.97},  Complex{5.59, -0.91},  Complex{5.56, -0.88},  Complex{5.49, -0.87},
            Complex{5.34, -0.87},  Complex{5.32, -0.87},  Complex{5.25, -0.93},  Complex{5.19, -1.05},
            Complex{5.19, -1.12},  Complex{5.15, -1.2},   Complex{5.15, -1.39},  Complex{5.15, -1.58},
            Complex{5.24, -1.79},  Complex{5.26, -1.87},  Complex{5.28, -1.95},  Complex{5.14, -1.96},
            Complex{5.05, -1.9},   Complex{4.91, -1.79},  Complex{4.82, -1.71},  Complex{4.73, -1.57},
            Complex{4.66, -1.45},  Complex{4.66, -1.35},  Complex{4.66, -1.18},  Complex{4.69, -1.13},
            Complex{4.79, -1.11},  Complex{4.88, -1.11},  Complex{4.98, -1.18},  Complex{5.04, -1.27},
            Complex{5.1, -1.42},   Complex{5.13, -1.51},  Complex{5.1, -1.36},   Complex{5.1, -1.25},
            Complex{5.1, -1.16},   Complex{5.09, -1.09},  Complex{5, -1.01},     Complex{4.95, -0.96},
            Complex{4.87, -0.91},  Complex{4.75, -0.87},  Complex{4.63, -0.85},  Complex{4.54, -0.85},
            Complex{4.49, -0.86},  Complex{4.41, -0.92},  Complex{4.36, -0.99},  Complex{4.3, -1.09},
            Complex{4.29, -1.12},  Complex{4.24, -1.09},  Complex{4.19, -1.03},  Complex{4.17, -1},
            Complex{4.11, -0.97},  Complex{4.05, -0.93},  Complex{3.98, -0.89},  Complex{3.93, -0.86},
            Complex{3.78, -0.83},  Complex{3.74, -0.83},  Complex{3.67, -0.87},  Complex{3.63, -0.93},
            Complex{3.59, -1},     Complex{3.57, -1.07},  Complex{3.5, -1.07},   Complex{3.47, -1.03},
            Complex{3.43, -0.98},  Complex{3.38, -0.94},  Complex{3.35, -0.91},  Complex{3.29, -0.89},
            Complex{3.14, -0.89},  Complex{3.04, -0.89},  Complex{2.99, -0.89},  Complex{2.95, -0.95},
            Complex{2.92, -1.03},  Complex{2.91, -1.11},  Complex{2.88, -1.16},  Complex{2.79, -1.07},
            Complex{2.75, -1},     Complex{2.7, -0.95},   Complex{2.65, -0.91},  Complex{2.62, -0.9},
            Complex{2.54, -0.87},  Complex{2.38, -0.87},  Complex{2.33, -0.87},  Complex{2.25, -0.93},
            Complex{2.19, -0.98},  Complex{2.1, -1.07},   Complex{2.06, -1.12},  Complex{2, -1.09},
            Complex{1.96, -1.04},  Complex{1.93, -1},     Complex{1.88, -0.97},  Complex{1.81, -0.91},
            Complex{1.71, -0.85},  Complex{1.6, -0.81},   Complex{1.55, -0.82},  Complex{1.48, -0.83},
            Complex{1.34, -0.87},  Complex{1.23, -0.94},  Complex{1.19, -0.99},  Complex{1.13, -1.14},
            Complex{1.12, -1.23},  Complex{1.1, -1.23},   Complex{1.06, -1.19},  Complex{1.02, -1.15},
            Complex{0.99, -1.12},  Complex{0.95, -1.1},   Complex{0.87, -1.06},  Complex{0.79, -1.07},
            Complex{0.66, -1.07},  Complex{0.58, -1.08},  Complex{0.54, -1.03},  Complex{0.53, -1},
            Complex{0.49, -0.96},  Complex{0.43, -0.91},  Complex{0.32, -0.87},  Complex{0.24, -0.87},
            Complex{0.08, -0.87},  Complex{-0.07, -0.87}, Complex{-0.1, -0.88},  Complex{-0.17, -0.94},
            Complex{-0.2, -0.99},  Complex{-0.12, -1.19}, Complex{-0.1, -1.32},  Complex{0.06, -1.76},
            Complex{0.21, -1.95},  Complex{0.37, -1.95},  Complex{0.46, -1.95},  Complex{0.55, -1.91},
            Complex{0.58, -1.84},  Complex{0.58, -1.71},  Complex{0.58, -1.6},   Complex{0.54, -1.46},
            Complex{0.5, -1.35},   Complex{0.45, -1.26},  Complex{0.39, -1.18},  Complex{0.32, -1.14},
            Complex{0.22, -1.08},  Complex{0.09, -1.04},  Complex{-0.02, -1.07}, Complex{-0.1, -1.14},
            Complex{-0.18, -1.03}, Complex{-0.22, -0.85}, Complex{-0.22, -0.78}, Complex{-0.28, -0.57},
            Complex{-0.33, -0.41}, Complex{-0.37, -0.28}, Complex{-0.38, -0.15}, Complex{-0.46, 0},
            Complex{-0.74, 0.03},  Complex{-0.47, -1.16}, Complex{-0.19, -2.15}, Complex{-0.98, -1.56},
            Complex{-1.06, -1.45}, Complex{-1.18, -1.35}, Complex{-1.31, -1.29}, Complex{-1.45, -1.22},
            Complex{-1.46, -1.25}, Complex{-1.73, -1.3},  Complex{-1.66, -1.52}, Complex{-1.63, -1.71},
            Complex{-1.62, -1.88}, Complex{-1.7, -1.96},  Complex{-1.89, -1.98}, Complex{-1.99, -1.94},
            Complex{-2.1, -1.81},  Complex{-2.21, -1.62}, Complex{-2.21, -1.48}, Complex{-2.21, -1.24},
            Complex{-2.18, -1.12}, Complex{-2.1, -1.07},  Complex{-2.01, -1.05}, Complex{-1.92, -1.07},
            Complex{-1.84, -1.12}, Complex{-1.78, -1.23}, Complex{-1.76, -1.26}, Complex{-1.46, -1.24},
            Complex{-1.38, -1.39}, Complex{-1.35, -1.56}, Complex{-1.31, -1.67}, Complex{-1.31, -1.76},
            Complex{-1.33, -1.92}, Complex{-1.38, -2.02}, Complex{-1.46, -2.08}, Complex{-1.57, -2.21},
            Complex{-1.65, -2.24}, Complex{-1.86, -2.24}, Complex{-2, -2.22},    Complex{-2.09, -2.17},
            Complex{-2.21, -2.07}, Complex{-2.3, -1.95},  Complex{-2.34, -1.91}, Complex{-2.71, -1.95}};
        // back to sanity
        Complex::ScaleBatch(Coordinates, 1);
        Coordinates = Complex::Interpolate(Coordinates, 0);
        FourierSeries F(Coordinates);
        const Arrow COM(F.Data[0].Amplitude, F.Data[0].Frequency, F.Data[0].Phase, Vec2D());
        // dont care abt 0th arrow, should be the new initP
        InitialPosition = COM.Tip(); // set Initial position to COM's tip
        F.BubbleSort();
        for (int i = 0; i < NumArrows + 1; i++)
        {
            if (i < F.Data.size() && F.Data[i].Frequency != 0)
            {
                AddArrow(Arrow(F.Data[i].Amplitude, F.Data[i].Frequency, F.Data[i].Phase, InitialPosition));
            }
        }
        DeltaTime = F.Data.size() / (2.0 * M_PI);
    }

    void Update()
    {
        if (!Train.empty())
        {
            for (size_t i = 0; i < Train.size(); i++)
            {
                Train[i].Theta += Train[i].Velocity / DeltaTime; // updates position over time
                /// TODO: fix these angle clamps jeez
                while (Train[i].Theta > 2 * M_PI)
                {
                    Train[i].Theta -= 2 * M_PI;
                }
                while (Train[i].Theta < -2 * M_PI)
                {
                    Train[i].Theta += 2 * M_PI;
                }

                if (i > 0)
                    Train[i].Position = Train[i - 1].Tip(); // ensures all positions are tip to tip
            }
            Path.push_back(Train[Train.size()].Tip());
        }
    }

    void Render()
    {
        /// TODO: add thickness
        const double ppm = 10;
        for (int i = 0; i < Train.size(); i++)
        {
            I.DrawLine(Train[i].Position * ppm, Train[i].Tip() * ppm, Colour(255, 255, 255));
            I.DrawStrokedCircle(Train[i].Position * ppm, Train[i].Length * ppm, Colour(255, 255, 255));
        }

        if (PenDown)
        {
            for (int i = 0; i < Path.size(); i++) // start @ 2nd to not worry abt vector end
            {
                // draw points on the path
                I.DrawSolidCircle(Path[i] * ppm, 3, Colour(255, 255, 255));
            }
        }
        I.ExportPPMImage();
        /// TODO: keep the paths in an old Image, only reset the arrow positions
        I.Blank();
    }
};

#endif