#ifndef FOURIER
#define FOURIER

#include "Complex.hpp"
#include "Utils.hpp"
#include <vector>

class FourierSeries
{
  public:
    /// TODO: make this better
    FourierSeries() = default;

    void DFT(const std::vector<Complex> &Input, const size_t TiD);

    void Sort(const size_t TiD);

    void BubbleSort(); // depracated in favour of Sort()

    struct FourierElements
    {
        FourierElements() = default;
        FourierElements(const Complex &c, const double F, const double P, const double A)
        {
            C = c;
            Frequency = F;
            Phase = P;
            Amplitude = A;
        }
        Complex C;
        double Frequency, Phase, Amplitude;
    };

    // comparison operator for fast sorting
    struct FourierCmp
    {
        inline bool operator()(const FourierElements &F1, const FourierElements &F2)
        {
            return (F1.Amplitude < F2.Amplitude);
        }
    };
    // shared among all instances (so all procs can share)
    static std::vector<FourierElements> Data;
};

#endif