#include "manga_library.hpp"
#include "server.hpp"

#include <thread>

#include <boost/locale.hpp>

int main(int argc, char **argv)
{
    boost::locale::generator gen;

    std::locale jp_locale(gen("ja_JP"));
    std::locale::global(jp_locale);

    mangapp::manga_library library({ L"H:/Manga/Manga"});

    mangapp::server server(1234, &library);

    std::thread server_thread([&server]()
    {
        server.start();
    });

    server_thread.join();

    return 0;
}
