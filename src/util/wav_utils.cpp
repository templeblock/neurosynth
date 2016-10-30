#include "wav_utils.hpp"

#include <vector>


namespace neurosynth
{
    double int2double_16(short s)
    {
        return double(s)/double(0x8000);
    }

    short double2int_16(double s)
    {
        return short(s * double(0x8000));
    }

    void load_wav(std::string& filename,
                  WavData& wav_data,
                  Logger& logger)
    {
        std::streambuf* buf;
        std::ifstream ifstream;

        if(filename == "-")
            buf = std::cin.rdbuf();
        else
        {
            ifstream.open(filename);
            buf = ifstream.rdbuf();
        }

        std::istream stream(buf);
        if(!stream)
            logger.warn("Cannot open file: " + filename);

        wav_data.samples.clear();

        short sample;
        while(stream >> sample)
            wav_data.samples.push_back(int2double_16(sample));
    }

    void st_fourier(WavData& wav_data,
                    StFourierData& st_fourier_data,
                    Logger& logger)
    {
        int input_size  = wav_data.samples.size();
        int output_size = input_size/2 + 1;
        fftw_complex* output_buf =
            (fftw_complex*)(fftw_malloc(output_size * sizeof(fftw_complex)));

        fftw_plan plan = fftw_plan_dft_r2c_1d(
            input_size,
            wav_data.samples.data(),
            output_buf,
            FFTW_ESTIMATE);

        fftw_execute(plan);
    }
}
