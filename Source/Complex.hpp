#ifndef COMPLEX
#define COMPLEX

#include "Utils.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Complex
{
  public:
    Complex() = default;
    Complex(const double r, double i)
    {
        Real = r;
        /// TODO: only negate y on rendering
        Imag = -i;
    }
    double Real, Imag; // real and imaginary components
    Complex operator*(const Complex &a) const
    {
        /// TODO: MAKE THIS INTO CLASS FUNCTION
        const double RealNew = a.Real * Real - a.Imag * Imag; // i^2 = -1
        const double ImagNew = a.Real * Imag + a.Imag * Real;
        return Complex{RealNew, ImagNew};
    }
    Complex operator*(const double S) const
    {
        return Complex{S * Real, S * Imag};
    }
    Complex operator+(const Complex &a) const
    {
        return Complex{Real + a.Real, Imag - a.Imag};
    }
    Complex operator-(const Complex &a) const
    {
        return Complex{Real - a.Real, Imag + a.Imag};
    }

    // assignment
    void operator+=(const Complex &a)
    {
        Real += a.Real;
        Imag -= a.Imag;
    }
    void operator-=(const Complex &a)
    {
        Real -= a.Real;
        Imag += a.Imag;
    }
    void operator/=(const Complex &a)
    {
        Real /= a.Real;
        Imag /= a.Imag;
    }
    void operator*=(const Complex &a)
    {
        Real *= a.Real;
        Imag *= a.Imag;
    }
    void operator*=(const double S)
    {
        Real *= S;
        Imag *= S;
    }
    void operator/=(const double S)
    {
        Real /= S;
        Imag /= S;
    }

    double Size() const
    {
        return std::sqrt(sqr(Real) + sqr(Imag));
    }

    double Angle() const
    {
        return std::atan2(Imag, Real);
    }

    static std::vector<Complex> Interpolate(std::vector<Complex> &Inputs, const size_t InterpAmnt)
    {
        /// TODO: optimize & clean & inline
        const int initSize = Inputs.size();
        std::vector<Complex> NewVec;
        for (int i = 0; i < initSize; i++)
        {
            NewVec.push_back(Inputs[i]);
            double deltaX, deltaY;
            if (i < initSize - 1)
            {
                deltaX = (Inputs[i + 1].Real - Inputs[i].Real) / (InterpAmnt + 1);
                deltaY = (Inputs[i + 1].Imag - Inputs[i].Imag) / (InterpAmnt + 1);
            }
            else
            {
                deltaX = (Inputs[0].Real - Inputs[i].Real) / (InterpAmnt + 1);
                deltaY = (Inputs[0].Imag - Inputs[i].Imag) / (InterpAmnt + 1);
            }
            for (int j = 1; j < InterpAmnt + 1; j++)
            {
                NewVec.push_back(Complex{Inputs[i].Real + deltaX * j, -(Inputs[i].Imag + deltaY * j)});
            }
        }
        return NewVec;
    }
    static std::vector<Complex> ScaleBatch(std::vector<Complex> &Inputs, const double scalar)
    {
        for (size_t i = 0; i < Inputs.size(); i++)
        {
            Inputs[i] *= scalar;
        }
        // operation operates in place, return inputs
        return Inputs;
    }

    static void ReadCSV(std::vector<Complex> &Output, const std::string &FilePath)
    {
        // create input stream to get file data
        std::ifstream InputCSV(FilePath);
        if (!InputCSV.is_open())
        {
            std::cout << "ERROR: could not open" << FilePath << std::endl;
            exit(1);
        }
        else
        {
            std::cout << "Reading " << FilePath << "..." << std::endl;
        }
        // read from file into Output vector
        std::string Tmp;
        const std::string Delim = ",";
        while (!InputCSV.eof())
        {
            InputCSV >> Tmp;
            if (InputCSV.bad() || InputCSV.fail())
                break;

            // parse line for the values
            std::string RealStr = Tmp.substr(0, Tmp.find(Delim));
            // offset +1 -1 for the commas before & after
            std::string ImagStr = Tmp.substr(Tmp.find(Delim) + 1, Tmp.size() - 1);
            if (RealStr.compare("dataX") == 0 || ImagStr.compare("dataY") == 0)
                continue; // ignore headers
            double Real = std::stod(RealStr);
            double Imag = std::stod(ImagStr);
            Output.push_back(Complex(Real, Imag));
        }
    }
};

#endif