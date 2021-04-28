#include "Drawing.hpp"
#include <array>
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
        NumIters = NI / P;
        NumThreads = P;
        // creating a new drawing per thread for into-the-future parallelism
        std::cout << "Running on " << FN << " for " << NI << " iterations with " << NA << " moons and " << P
                  << " threads at " << UpT << " updates per tick" << std::endl;
        InitializeDrawings(NA, UpT, WindowSize, FN);
    }
    std::vector<Drawing> Ds;
    bool RenderingMovie = true;
    size_t NumIters, NumThreads;

    void InitializeDrawings(const size_t NA, const size_t UpT, const Vec2D &WindowSize, std::string &FN)
    {
        for (size_t i = 0; i < NumThreads; i++)
        {
            Ds.push_back(Drawing(NA, true, i, NumThreads, UpT, WindowSize, FN));
        }
        std::array<std::vector<Complex>, 4> InputChunks;
#pragma omp parallel for num_threads(NumThreads)
        for (size_t i = 0; i < NumThreads; i++)
        {
            // accumulate all the file-io chunks
            InputChunks[i] = Ds[i].ReadInputFile();
        }
        std::cout << "... Done!" << std::endl;
        // sequentially join all the vectors together in *correct* order
        std::vector<Complex> Cumulative;
        for (size_t i = 0; i < NumThreads; i++)
        {
            std::vector<Complex> Chunk = InputChunks[i];
            Cumulative.insert(Cumulative.end(), Chunk.begin(), Chunk.end());
        }
#pragma omp parallel for num_threads(NumThreads)
        for (size_t i = 0; i < NumThreads; i++)
        {
            // run the fourierInit for all drawings (slow) in parallel
            Ds[i].FourierInit(Cumulative);
        }
    }

    void Run()
    {
        double ElapsedTime = 0;
        auto StartTime = std::chrono::system_clock::now();
        for (size_t i = 0; i < NumIters; i++)
        {
            ElapsedTime += Tick();
        }
        std::cout << "Epicycle updating took " << ElapsedTime << "s" << std::endl;
        auto EndTime = std::chrono::system_clock::now();
        std::chrono::duration<double> TotalTime = EndTime - StartTime;
        std::cout << "Total simulation took " << TotalTime.count() << "s" << std::endl;
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
    const size_t NumThreads = 8;
    const size_t NumIters = 200;
    const size_t UpdatesPerTick = 2;
    std::string FileName = "ai_dragonscs.csv";
    const Vec2D ScreenDim(1000, 1000);
    FourierDraw FD(NumArrows, NumThreads, NumIters, UpdatesPerTick, ScreenDim, FileName);
    FD.Run();
    return 0;
}
