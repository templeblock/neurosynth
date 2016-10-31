#include "wav_utils.hpp"

#include <cassert>
#include <vector>


namespace neurosynth
{

    double freq2mel(double s)
    {
        return 1125.0 * log(1.0 + s / 700.0);
    }

    double mel2freq(double s)
    {
        return 700.0 * (exp(s / 1125.0) - 1.0);
    }

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
            ifstream.open(filename, std::ios::binary);
            buf = ifstream.rdbuf();
        }

        std::istream stream(buf);
        if(!stream)
            logger.warn("Cannot open file: " + filename);

        wav_data.samples_l.clear();
        wav_data.samples_r.clear();

        short sample_l, sample_r;
        while(true) {
            stream.read((char*)&sample_l, sizeof(short));
            stream.read((char*)&sample_r, sizeof(short));
            if(!stream)
                break;
            wav_data.samples_l.push_back(int2double_16(sample_l));
            wav_data.samples_r.push_back(int2double_16(sample_r));
        }

        logger.info("Read " + std::to_string(wav_data.samples_l.size()) +
                    "/" + std::to_string(wav_data.samples_r.size()) +
                    " samples for L/R channel from: " + filename);
    }

    void dft(WavData& wav_data,
             DftData& dft_data,
             Logger& logger)
    {
        int input_size  = wav_data.samples_l.size();
        int output_size = input_size/2 + 1;

        dft_data.spectrum_l.resize(output_size);
        dft_data.spectrum_r.resize(output_size);

        fftw_plan plan = fftw_plan_dft_r2c_1d
            (input_size,
             wav_data.samples_l.data(),
             reinterpret_cast<fftw_complex*>(dft_data.spectrum_l.data()),
             FFTW_ESTIMATE);

        fftw_execute(plan);
        fftw_destroy_plan(plan);

        plan = fftw_plan_dft_r2c_1d
            (input_size,
             wav_data.samples_r.data(),
             reinterpret_cast<fftw_complex*>(dft_data.spectrum_r.data()),
             FFTW_ESTIMATE);

        fftw_execute(plan);
        fftw_destroy_plan(plan);
    }

    double triangular_window(double n, double N)
    {
        return 1.0 - std::abs((n-(N-1)/2) /
                              (N / 2));
    }

    double hann_window(double n, double N)
    {
        constexpr double pi = M_PI;
        return 0.5 * (1.0 - cos(2.0*pi*n/(N-1)));
    }

    void compute_spectral_energy(DftData& dft_data,
                                 double min_freq,
                                 double max_freq,
                                 double sample_rate,
                                 double& energy_l,
                                 double& energy_r)
    {
        size_t N = dft_data.spectrum_l.size();
        size_t min_frame = size_t(min_freq * 2.0 / sample_rate * N);
        size_t max_frame = size_t(max_freq * 2.0 / sample_rate * N);
        N = max_frame - min_frame;

        energy_l = 0.0;
        energy_r = 0.0;
        for(size_t frame = min_frame; frame < max_frame; frame++)
        {
            double window = triangular_window(frame - min_frame, N);
            double mag_coeff_l = std::abs(dft_data.spectrum_l[frame]);
            double mag_coeff_r = std::abs(dft_data.spectrum_r[frame]);
            energy_l += mag_coeff_l * mag_coeff_l * window;
            energy_r += mag_coeff_r * mag_coeff_r * window;
        }

        energy_l = log(1.0 + energy_l);
        energy_r = log(1.0 + energy_r);
    }

    void dft2stft(DftData& dft_data,
                  StftData& stft_data,
                  size_t num_coeff,
                  double min_freq,
                  double max_freq,
                  double sample_rate)

    {
        FreqVector<double> freq_vec_l(min_freq, max_freq);
        FreqVector<double> freq_vec_r(min_freq, max_freq);

        for(size_t i = 0; i < num_coeff; i++)
        {
            double window_min_freq = mel2freq
                (freq2mel(min_freq) +
                 (freq2mel(max_freq) - freq2mel(min_freq)) * i / num_coeff);
            double window_max_freq = mel2freq
                (freq2mel(min_freq) +
                 (freq2mel(max_freq) - freq2mel(min_freq)) * (i+1) / num_coeff);

            double energy_l, energy_r;
            compute_spectral_energy(dft_data,
                                    window_min_freq,
                                    window_max_freq,
                                    sample_rate,
                                    energy_l,
                                    energy_r);
            freq_vec_l.power.push_back(energy_l);
            freq_vec_r.power.push_back(energy_r);
        }

        stft_data.spectrum_l.emplace_back(std::move(freq_vec_l));
        stft_data.spectrum_r.emplace_back(std::move(freq_vec_r));
    }

    void stft(WavData& wav_data,
              StftData& stft_data,
              size_t window_size,
              size_t window_step,
              size_t num_coeff,
              double min_freq,
              double max_freq,
              double sample_rate,
              Logger& logger)
    {
        logger.info("Performing STFT with parameters: "
                    "Window size: " + std::to_string(window_size) +
                    " frames = " + std::to_string(window_size /
                                                  sample_rate * 1000.0) +
                    " ms; sample rate: " + std::to_string(sample_rate) +
                    "; frequency range: " + std::to_string(min_freq) +
                    "hz - " + std::to_string(max_freq) +
                    "hz; # coefficients: " + std::to_string(num_coeff));

        int input_size  = wav_data.samples_l.size();

        WavData window_data;
        window_data.samples_l.resize(window_size);
        window_data.samples_r.resize(window_size);

        for(size_t t = 0; t <= input_size - window_size; t += window_step)
        {
            for(size_t dt = 0; dt < window_size; dt++)
            {
                double window = hann_window(dt, window_size);
                window_data.samples_l[dt] = wav_data.samples_l[t+dt] * window;
                window_data.samples_r[dt] = wav_data.samples_r[t+dt] * window;
            }

            DftData dft_data;
            dft(window_data, dft_data, logger);
            dft2stft(dft_data, stft_data,
                     num_coeff, min_freq, max_freq, sample_rate);
        }
    }

    void load_stft(std::string& filename,
                   StftData& stft_data,
                   Logger& logger)
    {
        std::streambuf* buf;
        std::ifstream ifstream;

        if(filename == "-")
            buf = std::cin.rdbuf();
        else
        {
            ifstream.open(filename, std::ios::binary);
            buf = ifstream.rdbuf();
        }

        std::istream stream(buf);
        if(!stream)
            logger.warn("Cannot open file: " + filename);

        size_t num_coeff;
        double min_freq;
        double max_freq;
        stream.read((char*)&num_coeff, sizeof(num_coeff));
        stream.read((char*)&min_freq, sizeof(min_freq));
        stream.read((char*)&max_freq, sizeof(max_freq));

        while(true) {
            stft_data.spectrum_l.emplace_back();
            stft_data.spectrum_r.emplace_back();
            FreqVector<double>& freq_vec_l = stft_data.spectrum_l.back();
            FreqVector<double>& freq_vec_r = stft_data.spectrum_r.back();
            freq_vec_l.min_freq = min_freq;
            freq_vec_l.max_freq = max_freq;
            freq_vec_r.min_freq = min_freq;
            freq_vec_r.max_freq = max_freq;
            for(size_t c = 0; c < num_coeff; c++)
            {
                freq_vec_l.power.push_back(0);
                freq_vec_r.power.push_back(0);
                stream.read((char*)&freq_vec_l.power[c], sizeof(double));
                stream.read((char*)&freq_vec_r.power[c], sizeof(double));
            }
        }

        logger.info("Read " + std::to_string(stft_data.spectrum_l.size()) +
                    "/" + std::to_string(stft_data.spectrum_r.size()) +
                    " features for L/R channel(" + std::to_string(num_coeff) +
                    "dimensions, " + std::to_string(min_freq) +
                    " - " + std::to_string(min_freq) +
                    "frequency range) from: " + filename);
    }

    void save_stft(std::string& filename,
                   StftData& stft_data,
                   Logger& logger)
    {
        std::streambuf* buf;
        std::ofstream ofstream;

        if(filename == "-")
            buf = std::cout.rdbuf();
        else
        {
            ofstream.open(filename, std::ios::binary);
            buf = ofstream.rdbuf();
        }

        std::ostream stream(buf);
        if(!stream)
            logger.warn("Cannot open file: " + filename);

        assert(stft_data.spectrum_l.size() == stft_data.spectrum_r.size());

        if(stft_data.spectrum_l.empty())
        {
            logger.warn("Attempted to write 0 feats to: " + filename);
            return;
        }

        size_t num_coeff = stft_data.spectrum_l[0].power.size();
        double min_freq = stft_data.spectrum_l[0].min_freq;
        double max_freq = stft_data.spectrum_l[0].max_freq;
        stream.write((char*)&num_coeff, sizeof(num_coeff));
        stream.write((char*)&min_freq, sizeof(min_freq));
        stream.write((char*)&max_freq, sizeof(max_freq));

        std::cout << num_coeff << '\n';
        std::cout << min_freq << '\n';
        std::cout << max_freq << '\n';

        for(size_t t = 0; t < stft_data.spectrum_l.size(); t++)
        {
            FreqVector<double>& freq_vec_l = stft_data.spectrum_l[t];
            FreqVector<double>& freq_vec_r = stft_data.spectrum_r[t];
            assert(freq_vec_l.power.size() == num_coeff);
            assert(freq_vec_l.min_freq == min_freq);
            assert(freq_vec_l.max_freq == max_freq);
            assert(freq_vec_r.power.size() == num_coeff);
            assert(freq_vec_r.min_freq == min_freq);
            assert(freq_vec_r.max_freq == max_freq);
            for(size_t c = 0; c < num_coeff; c++)
            {
                std::cout << freq_vec_l.power[c] << ' ';
                std::cout << freq_vec_r.power[c] << '\n';

                stream.write((char*)&freq_vec_l.power[c], sizeof(double));
                stream.write((char*)&freq_vec_r.power[c], sizeof(double));
            }

            std::cout << "\n\n\n";
        }

        logger.info("Written " + std::to_string(stft_data.spectrum_l.size()) +
                    "/" + std::to_string(stft_data.spectrum_r.size()) +
                    " features for L/R channel (" + std::to_string(num_coeff) +
                    "dimensions, " + std::to_string(min_freq) +
                    " - " + std::to_string(min_freq) +
                    "frequency range) to: " + filename);
    }
}
