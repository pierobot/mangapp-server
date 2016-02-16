#include "manga_library.hpp"
#include "server.hpp"

#include <fstream>
#include <thread>

#include <json11/json11.hpp>

#include <boost/locale.hpp>

int main(int argc, char **argv)
{
    boost::locale::generator gen;

    std::locale jp_locale(gen("ja_JP"));
    std::locale::global(jp_locale);

    std::ifstream settings_file("settings.json");
    if (settings_file.good() == true)
    {
        std::string settings_str(std::istreambuf_iterator<char>(settings_file), (std::istreambuf_iterator<char>()));
        std::string error_str;

        json11::Json settings_json = json11::Json::parse(settings_str, error_str);
        if (settings_json != nullptr)
        {
            auto manga_settings = settings_json["manga"];
            mangapp::manga_library manga_library(manga_settings);

            std::cout << "Manga library serving a total of " << manga_library.size() << " items." << std::endl;

            mangapp::server server(1234, &manga_library);
            std::thread server_thread([&server]()
            {
                server.start();
            });

            server_thread.join();
        }
        else
            std::cout << "Unable to parse settings. Reason: " << error_str << std::endl;
    }
    else
        std::cout << "Unable to open settings.json" << std::endl;

    return 0;
}
