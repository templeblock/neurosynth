#ifndef NEUROSYNTH_WAV_UTILS_HPP
#define NEUROSYNTH_WAV_UTILS_HPP

#include "logger.hpp"

#include <complex>
#include <fftw3.h>
#include <fstream>
#include <iostream>
#include <string>


namespace neurosynth
{
    struct WavData
    {
        std::vector<double> samples_l;
        std::vector<double> samples_r;
    };

    struct DftData
    {
        std::vector<std::complex<double>> spectrum_l;
        std::vector<std::complex<double>> spectrum_r;
    };

    template<class T>
    struct FreqVector
    {
        FreqVector(double _min_freq = 0.0,
                   double _max_freq = 0.0)
            : min_freq(_min_freq),
              max_freq(_max_freq) {}

        FreqVector(FreqVector&& fv)
            : min_freq(fv.min_freq),
              max_freq(fv.max_freq),
              power(std::move(fv.power)) {}

        double min_freq;
        double max_freq;
        std::vector<T> power;
    };

    struct StftData
    {
        std::vector<FreqVector<double>> spectrum_l;
        std::vector<FreqVector<double>> spectrum_r;
    };

    double freq2mel(double s);

    double mel2freq(double s);

    double int2double_16(short s);

    short double2int_16(double s);

    void load_wav(std::string& filename,
                  WavData& wav_data,
                  Logger& logger);

    void dft(WavData& wav_data,
             DftData& dft_data,
             Logger& logger);

    void stft(WavData& wav_data,
              StftData& stft_data,
              size_t window_size,
              size_t window_step,
              size_t num_coeff,
              double min_freq,
              double max_freq,
              double sample_rate,
              Logger& logger);

    void load_stft(std::string& filename,
                   StftData& stft_data,
                   Logger& logger);

    void save_stft(std::string& filename,
                   StftData& stft_data,
                   Logger& logger);
}

#endif
