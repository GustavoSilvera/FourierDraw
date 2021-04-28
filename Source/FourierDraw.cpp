#include "Drawing.hpp"
#include <chrono>
#include <cstdlib>
#include <omp.h>
#include <string>
#include <vector>

class FourierDraw
{
  public:
    FourierDraw(const size_t NA, const size_t P, const size_t NI, const size_t UpT, const Vec2D &WindowSize,
                std::string &FN)
    {
        NumIters = NI;
        NumThreads = P;
        // creating a new drawing per thread for into-the-future parallelism
        for (size_t i = 0; i < NumThreads; i++)
        {
            Ds.push_back(Drawing(NA, true, i, P, UpT, WindowSize, FN));
        }
    }
    std::vector<Drawing> Ds;
    bool RenderingMovie = true;
    size_t NumIters, NumThreads;
    void Run()
    {
        double ElapsedTime = 0;
        for (size_t i = 0; i < NumIters; i++)
        {
            ElapsedTime += Tick();
        }
        std::cout << "Finished simulation! Took " << ElapsedTime << "s" << std::endl;
    }

    double Tick()
    {
        // Run our actual problem
        auto StartTime = std::chrono::system_clock::now();
#pragma omp parallel for num_threads(NumThreads)
        for (size_t i = 0; i < Ds.size(); i++)
        {
            Ds[i].Update();
        }
        auto EndTime = std::chrono::system_clock::now();
        std::chrono::duration<double> ElapsedTime = EndTime - StartTime;
        // draw to file
        if (RenderingMovie)
        {
#pragma omp parallel for num_threads(NumThreads)
            for (size_t i = 0; i < Ds.size(); i++)
            {
                Ds[i].Render();
            }
        }
        return ElapsedTime.count(); // return wall clock time diff
    }
};

int main()
{
    /// TODO: Add params for rendering and window size and other vars
    const size_t NumArrows = 5000;
    /// TODO: use numthreads in some way (probably rendering)
    const size_t NumThreads = 4;
    const size_t NumIters = 200;
    const size_t UpdatesPerTick = 2;
    std::string FileName = "scotty.csv";
    const Vec2D ScreenDim(1000, 1000);
    FourierDraw FD(NumArrows, NumThreads, NumIters, UpdatesPerTick, ScreenDim, FileName);
    FD.Run();
    return 0;
}
