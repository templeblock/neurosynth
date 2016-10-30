#include "parse-opt.hpp"

#include <iostream>
#include <fftw3.h>


int main(int argc, char** argv)
{
    using namespace neurosynth;
    using namespace std;

    ParseOpt parse_opt("Usage: wav2stf <options> [input] [output]\n"
                       "Input/Output stream can be - (stdin/stdout))");

    string logfile = get_working_dir() + "/log/wav2stf.log";
    parse_opt.register_opt("l|log", &logfile, false, "Log file path");
    parse_opt.parse(argc, argv);

    Logger logger(logfile);
    handle_error(logger, "Error");

    return 0;
}
