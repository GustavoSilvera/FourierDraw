#ifndef FOURIER
#define FOURIER

#include "Complex.hpp"
#include <omp.h>
#include <vector>

class FourierSeries
{
  public:
    /// TODO: make this better
    FourierSeries() = default;
    FourierSeries(const std::vector<Complex> &Input, const size_t NT)
    { // constructor as discrete fourier transform
      /// TODO: measure if this DFT is the slow part
#pragma omp parallel for num_threads(NT)
        for (size_t k = 0; k < Input.size(); k++)
        {
            Complex Sum(0, 0); // complex number final
            for (size_t n = 0; n < Input.size(); n++)
            {
                double Phi = (2 * M_PI * k * n) / Input.size(); // constant for angular velocity
                const Complex C(std::cos(Phi), -std::sin(Phi)); // cos and sin component
                Sum += Input[n] * C;                            // multiply cos & sin comp by original complex
            }
            Sum /= Input.size();
            const double Freq = -double(k);
            const double Phase = Sum.Angle();
            const double Amp = Sum.Size();
#pragma omp critical
            {
                // dosent matter where the data ends up since it will need to be sorted
                Data.push_back(FourierElements(Sum, Freq, Phase, Amp));
            }
        }
    }

    void Sort()
    {
        std::sort(Data.begin(), Data.end(), FourierCmp());
        std::reverse(Data.begin(), Data.end());
    }

    void BubbleSort()
    {
        /// NOTE: this is a naive sort that should not be used (O(n^2))
        FourierElements Tmp;
        for (int i = 0; i < Data.size(); i++)
        { // fwds
            for (int j = Data.size() - 1; j > i; j--)
            { // bkwds
                if (Data[j].Amplitude > Data[j - 1].Amplitude)
                {
                    Tmp = Data[j - 1];
                    Data[j - 1] = Data[j];
                    Data[j] = Tmp;
                }
            }
        }
    }

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
    std::vector<FourierElements> Data;
};

#endif