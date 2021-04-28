#include "Drawing.hpp"
#include <chrono>
#include <cstdlib>
#include <omp.h>
#include <string>
#include <vector>

class FourierDraw
{
  public:
    FourierDraw(const size_t NA, const size_t P, const double TB, const double DT, const Vec2D &WindowSize,
                std::string &FN)
    {
        Time = 0;
        TimeBudget = TB;
        DeltaTime = DT;
        D = Drawing(NA, DT, true, WindowSize, FN);
    }
    Drawing D;
    bool RenderingMovie = true;
    double Time, TimeBudget, DeltaTime;

    void Run()
    {
        double ElapsedTime = 0;
        while (Time <= TimeBudget)
        {
            ElapsedTime += Tick();
        }
        std::cout << "Finished simulation! Took " << ElapsedTime << "s" << std::endl;
    }

    double Tick()
    {
        // Run our actual problem
        auto StartTime = std::chrono::system_clock::now();
        const size_t Freq = 10; // does this many updates at once
        D.Update(Freq);
        auto EndTime = std::chrono::system_clock::now();
        std::chrono::duration<double> ElapsedTime = EndTime - StartTime;
        // draw to file
        if (RenderingMovie)
        {
            D.Render();
        }
        Time += DeltaTime;          // update simulator time
        return ElapsedTime.count(); // return wall clock time diff
    }
};

int main()
{
    /// TODO: Add params for rendering and window size and other vars
    const size_t NumArrows = 1000;
    /// TODO: use numthreads in some way (probably rendering)
    const size_t NumThreads = 4;
    const double MaxT = 45.0;
    const double DeltaT = 0.05;
    std::string FileName = "ai_dragonscs.csv";
    const Vec2D ScreenDim(1000, 1000);
    FourierDraw FD(NumArrows, NumThreads, MaxT, DeltaT, ScreenDim, FileName);
    FD.Run();
    return 0;
}
