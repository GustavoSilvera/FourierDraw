#include "Fourier.hpp"
#include <omp.h>

// initialize statics
std::vector<FourierSeries::FourierElements> FourierSeries::Data;

void FourierSeries::DFT(const std::vector<Complex> &Input, const size_t TiD)
{
    // constructor as discrete fourier transform
    if (TiD == 0)
        std::cout << "Starting DFT computation" << std::endl;
    for (size_t k = TiD; k < Input.size(); k += Params.Simulator.NumThreads)
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
#pragma omp barrier
    if (TiD == 0)
        std::cout << "Finished DFT computation" << std::endl;
}

void FourierSeries::Sort(const size_t TiD)
{
    if (TiD == 0)
    {
        /// TODO: use parallel (merge) sort
        // see https://codereview.stackexchange.com/questions/22744/multi-threaded-sort
        std::sort(Data.begin(), Data.end(), FourierCmp());
        std::reverse(Data.begin(), Data.end());
    }
#pragma omp barrier
    // make sure all threads reach this point simultaneously
    // so they all have the most up-to-date Data instance
}

void FourierSeries::BubbleSort()
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