#include "Drawing.hpp"
#include <chrono>
#include <cstdlib>
#include <omp.h>
#include <string>
#include <vector>

class FourierDraw
{
  public:
    FourierDraw(const size_t NA, const size_t P, const double TB, const double DT, const Vec2D &WindowSize)
    {
        Time = 0;
        TimeBudget = TB;
        DeltaTime = DT;
        D = Drawing(NA, DT, true, WindowSize);
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

        // Run our actual problem (boid computation)
        auto StartTime = std::chrono::system_clock::now();
        D.Update();
        auto EndTime = std::chrono::system_clock::now();
        std::chrono::duration<double> ElapsedTime = EndTime - StartTime;
        // draw to file
        if (RenderingMovie)
        {
            // Rendering is not part of our problem
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
    const size_t NumThreads = 4;
    const double MaxT = 8.0;
    const double DeltaT = 0.05;
    const Vec2D ScreenDim(1000, 1000);
    FourierDraw FD(NumArrows, NumThreads, MaxT, DeltaT, ScreenDim);
    FD.Run();
    return 0;
}
