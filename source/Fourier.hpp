#ifndef FOURIER
#define FOURIER

#include "Utils.hpp"
#include <vector>

class FourierSeries
{
  public:
    /// TODO: make this better
    FourierSeries() = default;
    FourierSeries(std::vector<Complex> &Input)
    {
        // constructor as discrete fourier transform
        for (size_t k = 0; k < Input.size(); k++)
        {
            Complex Sum(0, 0); // complex number final
            for (size_t n = 0; n < Input.size(); n++)
            {
                double Phi = (2 * M_PI * k * n) / N;            // constant for angular velocity
                const Complex C(std::cos(Phi), -std::sin(Phi)); // cos and sin component
                Sum += Input[n] * C;                            // multiply cos & sin comp by original complex
            }
            Sum /= Input.size();
            const double Freq = -double(k);
            const double Phase = Sum.Angle();
            const double Amp = Sum.Size();
            Data.push_back(FourierElements(Sum, Freq, Phase, Amp));
        }
    }

    void BubbleSort()
    {
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
    std::vector<FourierElements> Data;
};

#endif