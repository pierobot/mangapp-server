#include "manga_library.hpp"
#include "server.hpp"

#include <fstream>
#include <thread>
#include <mutex>

#include <json11/json11.hpp>

#include <boost/locale.hpp>
#include <boost/program_options.hpp>

#include <boost/algorithm/string.hpp>

void console_write_line(std::string const & function_name, std::string const & message)
{
    static std::mutex mutex_cout;
    std::lock_guard<std::mutex> lock(mutex_cout);

    std::cout << function_name << " - " << message << std::endl;
}

static bool verify_arguments(boost::program_options::variables_map const & args,
                             boost::program_options::options_description const & options)
{
    do
    {
        if (args.count("help"))
        {
            std::cout << options << std::endl;
            break;
        }

        if (args.count("settings-file") == 0)
        {
            std::cout << "No JSON file containing library settings provided." << std::endl;
            break;
        }

        return true;

    } while (true);
    
    return false;
}

int main(int argc, char **argv)
{   
    boost::locale::generator gen;
    std::locale::global(gen.generate(""));
    boost::filesystem::path::imbue(std::locale());
    // Set up our command line arguments
    boost::program_options::options_description options("Supported options");
    options.add_options()
        ("help", "Produce help message")
        ("settings-file", boost::program_options::value<std::string>(), "Path to a json file that contains the library settings.");
    // Parse the command line arguments
    boost::program_options::variables_map args;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), args);
    // Stop execution if any parameters are invalid or we're missing any
    if (verify_arguments(args, options) == false)
        return 1;  

    std::ifstream settings_file(args["settings-file"].as<std::string>());
    if (settings_file.good() == true)
    {
        std::string settings_str(std::istreambuf_iterator<char>(settings_file), (std::istreambuf_iterator<char>()));
        std::string error_str;

        json11::Json settings_json = json11::Json::parse(settings_str, error_str);
        if (settings_json != nullptr)
        {
            mangapp::server server(settings_json);
            server.start();
        }
        else
            std::cout << "Unable to parse settings. Reason: " << error_str << std::endl;
    }
    else
        std::cout << "Unable to open settings.json" << std::endl;

    return 0;
}
