#ifndef FILE_ENUMERATION_HPP
#define FILE_ENUMERATION_HPP

#include <iostream>
#include <functional>
#include <string>

#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

enum file_search_flags
{
    FlagDirectory = 1,
    FlagFile
};

inline void enumerate_files(std::wstring const & path,
    file_search_flags flags,
    bool recursive,
    std::function<void(std::wstring const &, file_search_flags flag)> callback_fn)
{
    using namespace boost::filesystem;

    if (callback_fn == nullptr)
        return;

    boost::system::error_code ec;
    for (auto current = directory_iterator(path, ec); current != directory_iterator(); ++current)
    {
        auto const & path_str = current->path().generic_wstring();

        if (is_directory(path_str) == true)
        {
            if (recursive == true)
            {
                enumerate_files(path_str, flags, recursive, callback_fn);
            }
            else if (flags & file_search_flags::FlagDirectory)
            {
                callback_fn(path_str, file_search_flags::FlagDirectory);
            }
        }
        else if (flags & file_search_flags::FlagFile)
        {
            callback_fn(path_str, file_search_flags::FlagFile);
        }
    }

    if (ec)
    {
        std::cout << ec.value() << " " << ec.message() << std::endl;
    }
}

#endif