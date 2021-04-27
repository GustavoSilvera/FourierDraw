#ifndef DRAWING
#define DRAWING

#include "Arrow.hpp"
#include "Complex.hpp"
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

    void FourierInit()
    {
        /// Initializes all the arrows via the DFT on the complex coordinates
        std::vector<Complex> ImgPixels;
        const std::string FilePath = "Images/Plaid_norm.csv";
        Complex::ReadCSV(ImgPixels, FilePath);
        // Complex::ScaleBatch(ImgPixels, 1);
        // ImgPixels = Complex::Interpolate(ImgPixels, 0);
        FourierSeries F(ImgPixels);
        const Arrow COM(F.Data[0].Amplitude, F.Data[0].Frequency, F.Data[0].Phase, Vec2D());
        // dont care abt 0th arrow, should be the new initP
        InitialPosition = COM.Tip(); // set Initial position to COM's tip
        F.BubbleSort();
        for (int i = 0; i < std::min(NumArrows, F.Data.size()); i++)
        {
            Train.push_back(Arrow(F.Data[i].Amplitude, F.Data[i].Frequency, F.Data[i].Phase, InitialPosition));
        }
        DeltaTime = (2.0 * M_PI) / F.Data.size();
    }

    void Update(const size_t Freq)
    {
        for (size_t i = 0; i < Freq; i++)
        {
            Arrow Last;
            size_t ArrowIndex = 0;
            for (Arrow &A : Train)
            {
                A.Theta += A.Velocity * DeltaTime; // updates position over time
                /// TODO: fix these angle clamps jeez
                if (A.Theta > TWOPI)
                    A.Theta -= TWOPI;
                if (A.Theta < -TWOPI)
                    A.Theta += TWOPI;

                if (ArrowIndex > 0)
                {
                    A.Position = Last.Tip(); // ensures all positions are tip to tip
                }
                Last = A;
                ArrowIndex++;
            }
            Path.push_back(Train[Train.size() - 1].Tip());
        }
    }

    void Render()
    {
        /// TODO: add thickness
        const double ppm = 500;                                      // for normalized
        const Vec2D Center = Vec2D(I.MaxWidth / 2, I.MaxHeight / 2); // middle of the window
        for (size_t i = 0; i < Train.size(); i++)
        {
            const Vec2D Origin = Center + Train[i].Position * ppm;
            const Vec2D End = Center + Train[i].Tip() * ppm;
            I.DrawLine(Origin, End, Colour(255, 255, 255));
            I.DrawStrokedCircle(Origin, Train[i].Length * ppm, Colour(255, 255, 255));
        }

        if (PenDown)
        {
            for (size_t i = 0; i < Path.size(); i++) // start @ 2nd to not worry abt vector end
            {
                // draw points on the path
                I.DrawSolidCircle(Center + Path[i] * ppm, 3, Colour(255, 0, 0));
                if (i > 1)
                {
                    I.DrawLine(Center + Path[i] * ppm, Center + Path[i - 1] * ppm, Colour(0, 0, 255));
                }
            }
        }
        I.ExportPPMImage();
        /// TODO: keep the paths in an old Image, only reset the arrow positions
        I.Blank();
    }
};

#endif