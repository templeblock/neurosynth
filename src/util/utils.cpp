#include "utils.hpp"

#include <unistd.h>


namespace std
{
    string to_string(char val)
    {
        return string(1, val);
    }

    wstring to_wstring(char val)
    {
        return wstring(1, val);
    }
}

namespace neurosynth
{
    std::string join(const std::string& delim,
                     const std::vector<std::string>& vec)
    {
        if(vec.empty())
            return "";

        std::string result = vec[0];;
        for(uint i = 1; i < vec.size(); i++)
            result += delim + vec[i];

        return result;
    }

    std::string join(const char delim,
                     const std::vector<std::string>& vec)
    {
        std::string delim_str = std::to_string(delim);
        return join(delim_str, vec);
    }

    std::vector<std::string> split(const std::string& delim,
                                   const std::string& str,
                                   bool skip_empty)
    {
        std::vector<std::string> result;

        size_t start = 0;
        size_t end;

        while(true)
        {
            //find next delimiter postition
            end = str.find_first_of(delim, start);
            if(end == std::string::npos)
                end = str.size();

            //check before emiting empty substring
            if(start != end || !skip_empty)
                result.emplace_back(str.substr(start, end-start));

            if(end == str.size())
                break;

            start = end+1;
        }

        return result;
    }

    std::vector<std::string> split(const char delim,
                                   const std::string& str,
                                   bool skip_empty)
    {
        std::string delim_str = std::to_string(delim);
        return split(delim_str, str, skip_empty);
    }

    std::string replace_global(const std::string& str,
                               const std::string& from,
                               const std::string& to)
    {
        std::string result;
        std::string::const_iterator end = str.end();
        std::string::const_iterator pos = str.begin();
        std::string::const_iterator next =
            std::search(pos, end, from.begin(), from.end());

        while(next != end)
        {
            result.append(pos, next);
            result.append(to);
            pos = next + from.size();
            next = std::search(pos, end, from.begin(), from.end());
        }
        result.append(pos, next);
        return result;
    }

    void handle_errno(ssize_t result, Logger& logger,
                      std::string message)
    {
        if(result == -1)
        {
            message += std::string(" - error: ") + strerror(errno);
            logger.err(message);
            std::cerr << message << "\n";
            std::exit(1);
        }
    }

    void handle_error(Logger& logger,
                      std::string message)
    {
        logger.err(message);
        std::cerr << message << "\n";
        std::exit(1);
    }

    std::string get_working_dir()
    {
        char* c_cwd = get_current_dir_name();
        std::string cwd(c_cwd);
        free(c_cwd);
        return cwd;
    }
}
