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
    Drawing(const size_t NA, const bool PD, const size_t TiD, const size_t NT, const int UpT, const Vec2D &WindowSize,
            std::string &FN)
    {
        NumArrows = NA;
        PenDown = PD;
        FileName = FN;
        ThreadID = TiD;  // unique thread ID per drawing
        NumThreads = NT; // number of threads in the program
        UpdatesPerTick = UpT;
        I = I.Init(WindowSize, ThreadID, NumThreads);
        InitialPosition = Vec2D(0, 0); // WindowSize / 2;
        // initialize all the arrows from the fourier transform
    }

    size_t NumArrows, ThreadID, NumThreads, UpdatesPerTick;
    double DeltaTime;
    bool PenDown;
    std::string FileName;
    Image I;

    std::vector<Arrow> Train; // Train of arrows
    std::vector<Vec2D> Path;  // path drawn by final arrow
    Vec2D InitialPosition;

    std::vector<Complex> ReadInputFile()
    {
        // each thread reads a "chunk" of the input to divy-up the work
        std::vector<Complex> ImgPixels;
        const std::string FilePath = "Data/" + FileName;
        Complex::ReadCSV(ImgPixels, FilePath, ThreadID, NumThreads);
        return ImgPixels;
    }

    void FourierInit(std::vector<Complex> &ImgPixels)
    {
        /// Initializes all the arrows via the DFT on the complex coordinates
        // Complex::ScaleBatch(ImgPixels, 1);
        // ImgPixels = Complex::Interpolate(ImgPixels, 0);
        FourierSeries F(ImgPixels, NumThreads);
        const Arrow COM(F.Data[0].Amplitude, F.Data[0].Frequency, F.Data[0].Phase, Vec2D());
        // dont care abt 0th arrow, should be the new initP
        InitialPosition = COM.Tip(); // set Initial position to COM's tip
        F.Sort();
        for (int i = 0; i < std::min(NumArrows, F.Data.size()); i++)
        {
            Train.push_back(Arrow(F.Data[i].Amplitude, F.Data[i].Frequency, F.Data[i].Phase, InitialPosition));
        }
        DeltaTime = (2.0 * M_PI) / F.Data.size();
        OffsetByThread();
    }

    void OffsetByThread()
    {
        for (size_t i = 0; i < ThreadID; i++)
        {
            UpdateOnce();
        }
    }

    void UpdateOnce()
    {
        for (size_t i = 0; i < UpdatesPerTick; i++)
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

    void Update()
    {
        for (size_t i = 0; i < NumThreads; i++)
        {
            UpdateOnce();
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
        I.ExportPPMImage(ThreadID);
        /// TODO: keep the paths in an old Image, only reset the arrow positions
        I.Blank();
    }
};

#endif