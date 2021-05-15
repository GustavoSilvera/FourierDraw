#ifndef UTILS
#define UTILS

#include <cmath> // sqrt
#include <cstdio>
#include <fstream>
#include <iostream>

#define PI 3.14159265359
#define TWOPI 6.28318530718

inline double sqr(const double a)
{
    return a * a;
}
static inline double toDeg(const double rad)
{
    return (rad * 180.0 / PI);
}
static inline double toRad(const double deg)
{
    return (deg * PI / 180.0);
}
static inline double clamp(const double min, const double max, const double i)
{
    if (i > min)
    {
        if (i < max)
        {
            return i;
        }
        return max;
    }
    return min;
}
inline bool stob(const std::string &s)
{
    // assuming s is either "true" or "false"
    return (s.at(0) == 't');
}
//////////// :PARAMS: //////////////

struct SimulatorParamsStruct
{
    std::string FileName;
    size_t NumThreads, NumArrows, NumIters;
    size_t UpdatesPerTick;
    bool Render;
};

struct ImageParamsStruct
{
    size_t WindowX, WindowY;
};

struct ParamsStruct
{
    SimulatorParamsStruct Simulator;
    ImageParamsStruct Image;
};

// global params (extern for multiple .o files)
extern ParamsStruct Params;

inline void ParseParams(const std::string &FilePath)
{
    // create input stream to get file data
    std::ifstream Input(FilePath);
    if (!Input.is_open())
    {
        std::cout << "ERROR: could not open \"" << FilePath << "\"" << std::endl;
        exit(1);
    }
    std::cout << "Reading params from " << FilePath << std::endl;

    // read from file into params
    std::string Tmp;
    const std::string Delim = "=";
    while (!Input.eof())
    {
        Input >> Tmp;
        if (Input.bad() || Input.fail())
            break;
        if (Tmp.at(0) == '[' || Tmp.at(0) == '#' || Tmp.at(0) == ';') // ignoring labels & comments
            continue;
        std::string ParamName = Tmp.substr(0, Tmp.find(Delim));
        std::string ParamValue = Tmp.substr(Tmp.find(Delim) + 1, Tmp.size());
        if (!ParamName.compare("input_image"))
            Params.Simulator.FileName = ParamValue;
        else if (!ParamName.compare("num_iters"))
            Params.Simulator.NumIters = std::stoi(ParamValue);
        else if (!ParamName.compare("num_threads"))
            Params.Simulator.NumThreads = std::stoi(ParamValue);
        else if (!ParamName.compare("num_arrows"))
            Params.Simulator.NumArrows = std::stod(ParamValue);
        else if (!ParamName.compare("updates_per_tick"))
            Params.Simulator.UpdatesPerTick = std::stoi(ParamValue);
        else if (!ParamName.compare("render"))
            Params.Simulator.Render = stob(ParamValue);
        else if (!ParamName.compare("window_x"))
            Params.Image.WindowX = std::stoi(ParamValue);
        else if (!ParamName.compare("window_y"))
            Params.Image.WindowY = std::stoi(ParamValue);
        else
            continue;
    }
}

#endif
