#ifndef NEUROSYNTH_LOGGER_HPP
#define NEUROSYNTH_LOGGER_HPP

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>


namespace neurosynth
{
    class Logger
    {
    public:
        Logger(const std::string& filename)
            : m_filename(filename)
        {
            boost::filesystem::path path(filename);
            boost::filesystem::create_directories(path.parent_path());

            m_file_stream.open(filename.c_str(), std::ios_base::out);
        }

        ~Logger() {}

        void info(const std::string& message)
        {
            if(m_file_stream)
            {
                m_file_stream << "INFO: " << message << "\n";
                m_file_stream.flush();
            }
        }

        void warn(const std::string& message)
        {
            if(m_file_stream)
            {
                m_file_stream << "WARNING: " << message << "\n";
                m_file_stream.flush();
            }
        }

        void err(const std::string& message)
        {
            if(m_file_stream)
            {
                m_file_stream << "ERROR: " << message << "\n";
                m_file_stream.flush();
            }
        }

    private:
        std::string   m_filename;
        std::ofstream m_file_stream;
    };
}

#endif
