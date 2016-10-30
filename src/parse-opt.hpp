#ifndef HMGEN_PARSE_OPT_HPP
#define HMGEN_PARSE_OPT_HPP

#include "types.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>


namespace neurosynth
{
    class ParseOpt
    {
    public:
        explicit ParseOpt(const std::string usage)
            : m_usage(usage)
        {
        }

        ~ParseOpt() {}

        //names   - names of an option
        //          alterantive names can be separated by '|'
        //          for example "v|verbose"
        //pointer - place in memory where an opt will be stored
        //          after parse()
        //is_flag - unlike normal options
        //          flags doesn't need arguments
        //          for example --verbose vs --log=./log...
        //doc     - docstring that will be printed
        //          after options such as --help etc.
        void register_opt(const std::string names,
                          void*             pointer,
                          bool              is_flag,
                          const std::string doc)
        {
            std::vector<std::string> names_vec =
                split('|', names, true);

            for(std::string name : names_vec)
            {
                if(is_flag)
                {
                    //default flag value is false
                    (*(bool*)pointer) = false;

                    m_flag_names.push_back(name);
                    m_flag_pointers.push_back((bool*)pointer);
                }
                else
                {
                    m_opt_names.push_back(name);
                    m_opt_pointers.push_back((std::string*)pointer);
                }
            }

            if(is_flag)
            {
                m_flag_names_readable.push_back("--" + join(" --", names_vec));
                m_flag_docs.push_back(doc);
            }
            else
            {
                m_opt_names_readable.push_back("--" + join(" --", names_vec));
                m_opt_docs.push_back(doc);
            }
        }

        //call parse after you registered all options!
        void parse(int argc, char** argv)
        {
            std::string key, value;
            for(int i = 1; i < argc; i++)
            {
                std::string arg = argv[i];
                size_t pos = 0;

                if(arg.size() > pos && arg[pos] == '-') pos++;
                if(arg.size() > pos && arg[pos] == '-') pos++;

                //no '-' prefix - positional option
                if(pos == 0)
                {
                    parse_positional(arg);
                }
                //'-' prefix - short option
                else if(pos == 1)
                {
                    //treat bare '-' as a positional option
                    if(arg.size() < 2)
                    {
                        parse_positional(arg);
                    }

                    //parse sequential short flags
                    uint flag_pos;
                    for(flag_pos = 1;
                        flag_pos < arg.size();
                        flag_pos++)
                    {
                        key = std::to_string(arg[flag_pos]);
                        if(is_flag(key)) parse_flag(key);
                        else break;
                    }

                    //there is non-flag option left
                    if(flag_pos < arg.size())
                    {
                        key = std::to_string(arg[flag_pos]);
                        value = arg.substr(flag_pos+1);
                        if(flag_pos+1 == arg.size()
                           && (++i) != argc)
                            value = argv[i];
                        parse_option(key, value);
                    }
                }
                //'--' prefix - long option
                else if(pos == 2)
                {
                    key = arg.substr(pos);
                    size_t equal_sign_pos = key.find_first_of('=');
                    //if '--key='
                    if(equal_sign_pos != std::string::npos)
                    {
                        value = key.substr(equal_sign_pos+1);
                        key   = key.substr(0, equal_sign_pos);
                        parse_option(key, value);
                    }
                    //there is no '=' sign
                    else if(!key.empty())
                    {
                        if(is_flag(key))
                            parse_flag(key);
                        else
                        {
                            value = "";
                            if((++i) != argc)
                                value = argv[i];

                            if(value.empty()) parse_flag(key);
                            else parse_option(key, value);
                        }
                    }
                    //after '--' treat args as positional
                    else
                    {
                        for(i++; i < argc; i++)
                        {
                            arg = argv[i];
                            parse_positional(arg);
                        }
                    }
                }
            }
        }

        //prints nicely formatted usage
        void print_usage()
        {
            //print usage
            std::cout << m_usage;

            //print flags
            std::cout << "\nFlags:\n";
            for(uint i = 0; i < m_flag_names_readable.size(); i++)
            {
                const std::string& option = m_flag_names_readable[i];
                size_t max_option_len = get_max_option_len();
                std::string blank_space_opt(max_option_len+6-
                                            option.size()-2,
                                            ' ');
                std::string blank_space_full(max_option_len+4, ' ');

                std::cout << option
                          << blank_space_opt
                          << replace_global(m_flag_docs[i],
                                            "\n",
                                            "\n"+blank_space_full)
                          << "\n\n";
            }

            //print options
            std::cout << "\nOptions:\n";
            for(uint i = 0; i < m_opt_names_readable.size(); i++)
            {
                const std::string& option = m_opt_names_readable[i];
                size_t max_option_len = get_max_option_len();
                std::string blank_space_opt(max_option_len+6-
                                            option.size()-2,
                                            ' ');
                std::string blank_space_full(max_option_len+4, ' ');

                std::cout << option
                          << blank_space_opt
                          << replace_global(m_opt_docs[i],
                                            "\n",
                                            "\n"+blank_space_full)
                          << "\n\n";
            }
        }

    private:
        const std::string m_usage;

        std::vector<std::string>  m_flag_names;
        std::vector<std::string>  m_flag_names_readable;
        std::vector<bool*>        m_flag_pointers;
        std::vector<std::string>  m_flag_docs;

        std::vector<std::string>  m_opt_names;
        std::vector<std::string*> m_opt_pointers;
        std::vector<std::string>  m_opt_names_readable;
        std::vector<std::string>  m_opt_docs;

        std::vector<std::string>  m_positional;

        void parse_flag(const std::string& key)
        {
            //display help flag is 'help'
            if(key == "help")
            {
                print_usage();
                exit(0);
            }

            //exit if key was not registered as flag
            if(!is_flag(key))
            {
                if(is_option(key))
                {
                    std::cerr << "Option -- '" + key
                              << "' requires an argument\n"
                              << "Try '--help' for more information.\n";
                }
                else
                {
                    std::cerr << "Invalid option -- '" + key + "'\n"
                              << "Try '--help' for more information.\n";
                }
                exit(1);
            }

            //find flag's index
            size_t flag_index = 0;
            while(key != m_flag_names[flag_index])
                flag_index++;

            //set flag's value to true
            *(m_flag_pointers[flag_index]) = true;
        }

        void parse_option(const std::string& key,
                          const std::string& value)
        {
            //exit if key was not registered as option
            if(!is_option(key))
            {
                if(is_flag(key))
                {
                    std::cerr << "Option '--" + key
                              << "' doesn't allow an argument\n"
                              << "Try '--help' for more information.\n";
                }
                else
                {
                    std::cerr << "Invalid option -- '" + key + "'\n"
                              << "Try '--help' for more information.\n";
                }
                exit(1);
            }

            //find opt's index
            size_t opt_index = 0;
            while(key != m_opt_names[opt_index])
                opt_index++;

            *(m_opt_pointers[opt_index]) = value;
        }

        void parse_positional(const std::string& value)
        {
            m_positional.push_back(value);
        }

        //checks if 'name' is registered as a flag
        bool is_flag(const std::string& name)
        {
            auto pos = std::find(m_flag_names.begin(),
                                 m_flag_names.end(),
                                 name);

            return (pos != m_flag_names.end());
        }

        //checks if 'name' is registered as an option
        bool is_option(const std::string& name)
        {
            auto pos = std::find(m_opt_names.begin(),
                                 m_opt_names.end(),
                                 name);

            return (pos != m_opt_names.end());
        }

        //get length of the biggest option
        //for formatting purposes
        size_t get_max_option_len()
        {
            size_t result = 0;
            for(const std::string& name : m_flag_names_readable)
                result = std::max<>(result, name.size());
            for(const std::string& name : m_opt_names_readable)
                result = std::max<>(result, name.size());
            return result;
        }
    };
}

#endif
