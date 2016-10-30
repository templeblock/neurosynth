#include "util/parse-opt.hpp"
#include "util/wav_utils.hpp"

#include <iostream>


int main(int argc, char** argv)
{
    using namespace neurosynth;
    using namespace std;

    ParseOpt parse_opt("Usage: wav2stf <options> [input] [output]\n"
                       "Input/Output stream can be - (stdin/stdout))");

    string logfile = get_working_dir() + "/log/wav2stf.log";
    parse_opt.register_opt("l|log", &logfile, false, "Log file path");
    parse_opt.parse(argc, argv);

    string input_fn  = parse_opt.get_positional(0);
    string output_fn = parse_opt.get_positional(1);

    cout << "Executing wav2stf with log file: " +
        logfile +
        ", input: " + input_fn +
        ", output: " + output_fn + "\n";

    Logger logger(logfile);

    WavData wav_data;
    StFourierData st_fourier_data;
    load_wav(input_fn, wav_data, logger);
    st_fourier(wav_data, st_fourier_data, logger);

    return 0;
}
