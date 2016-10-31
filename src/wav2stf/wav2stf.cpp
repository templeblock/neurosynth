#include "util/parse-opt.hpp"
#include "util/wav_utils.hpp"

#include <iostream>


int main(int argc, char** argv)
{
    using namespace neurosynth;
    using namespace std;

    ParseOpt parse_opt("Usage: wav2stf <options> [input] [output]\n"
                       "Input/Output stream can be - (stdin/stdout))");

    string sample_rate_str;
    size_t sample_rate = 44100;
    string logfile = get_working_dir() + "/log/wav2stf.log";
    parse_opt.register_opt("l|log", &logfile, false,
                           "Log file path");
    parse_opt.register_opt("r|rate", &sample_rate_str, false,
                           "Sample rate of audio (default 44100)");
    parse_opt.parse(argc, argv);

    if(!sample_rate_str.empty())
        sample_rate = stoi(sample_rate_str);

    string input_fn  = parse_opt.get_positional(0);
    string output_fn = parse_opt.get_positional(1);

    cout << "Executing wav2stf with log file: " +
        logfile +
        ", input: " + input_fn +
        ", output: " + output_fn + "\n";

    Logger logger(logfile);

    WavData wav_data;
    StftData stft_data;
    load_wav(input_fn, wav_data, logger);
    stft(wav_data, stft_data,
         2204, // 50ms
         1102, // move by 25ms
         88,   // # of frequency frames
         25,   // minimum 25hz
         4200, // maximum 4200hz
         sample_rate,
         logger);
    save_stft(output_fn, stft_data, logger);

    return 0;
}
