#ifndef NEUROSYNTH_WAV_UTILS_HPP
#define NEUROSYNTH_WAV_UTILS_HPP

#include "logger.hpp"

#include <fftw3.h>
#include <fstream>
#include <iostream>
#include <string>


namespace neurosynth
{
    struct WavData
    {
        std::vector<double> samples;
    };

    struct StFourierData
    {

    };

    double int2double_16(short s);
    short  double2int_16(double s);
    void load_wav(std::string& filename,
                  WavData& wav_data,
                  Logger& logger);
    void st_fourier(WavData& wav_data,
                    StFourierData& st_fourier_data,
                    Logger& logger);
}

#endif
