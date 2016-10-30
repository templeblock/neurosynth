#ifndef NEUROSYNTH_UTILS_HPP
#define NEUROSYNTH_UTILS_HPP

#include "logger.hpp"

#include <algorithm>
#include <cstdlib>
#include <errno.h>
#include <string>
#include <string.h>
#include <vector>


namespace std
{
    string to_string(char val);
    wstring to_wstring(char val);
}

namespace neurosynth
{
    std::string join(const std::string& delim,
                     const std::vector<std::string>& vec);

    std::string join(const char delim,
                     const std::vector<std::string>& vec);

    std::vector<std::string> split(const std::string& delim,
                                   const std::string& str,
                                   bool skip_empty = false);

    std::vector<std::string> split(const char delim,
                                   const std::string& str,
                                   bool skip_empty = false);

    std::string replace_global(const std::string& str,
                               const std::string& from,
                               const std::string& to);

    void handle_errno(ssize_t result, Logger& logger,
                      std::string message);

    void handle_error(Logger& logger,
                      std::string message);

    std::string get_working_dir();

    template<class T>
    bool is_power2(T t)
    {
        if(t == 0)
            return false;
        while(t != 1)
        {
            if(t % 2)
                return false;
            t = t/2;
        }
        return true;
    }
}

#endif
