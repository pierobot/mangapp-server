#include "archive_rar.hpp"

#include <iterator>

using namespace mangapp;

entry_rar::entry_rar(std::wstring rar_path, 
                     std::wstring name, 
                     bool is_dir, 
                     uint64_t size, 
                     uint64_t index) :
    m_rar_path(rar_path),
    m_name(name),
    m_is_dir(is_dir),
    m_size(size),
    m_index(index)
{
}

entry_rar::~entry_rar()
{
}

std::string const entry_rar::contents() const
{
    std::string rar_contents;

    RARHeaderDataEx rar_header = { 0 };
    RAROpenArchiveDataEx arc_data = { 0 };
    std::wstring rar_path(m_rar_path);
    arc_data.ArcNameW = &rar_path[0];
    arc_data.OpenMode = RAR_OM_EXTRACT;

    HANDLE rar_handle = RAROpenArchiveEx(&arc_data);
    // Were we able to open the rar archive
    if (rar_handle != nullptr)
    {
        // Yes, supply our callback to unrar lib
        RARSetCallback(rar_handle, rar_callback, reinterpret_cast<LPARAM>(&rar_contents));
        // Iterate through all the entries
        for (uint64_t entry_index = 0; RARReadHeaderEx(rar_handle, &rar_header) == ERAR_SUCCESS; entry_index++)
        {
            // If the entry index matches ours, extract the contents
            if (entry_index == m_index)
            {
                // Using RAR_TEST instead of RAR_EXTRACT will avoid extracting to disk
                RARProcessFileW(rar_handle, RAR_TEST, nullptr, nullptr);
            }
            else
                RARProcessFileW(rar_handle, RAR_SKIP, nullptr, nullptr);
        }

        RARCloseArchive(rar_handle);
    }
    else
    {
        // No, something went wrong - add loggin?
    }

    return rar_contents;
}

int CALLBACK entry_rar::rar_callback(UINT msg, LPARAM user_data, LPARAM p1, LPARAM p2)
{
    // Get our pointer to the rar contents
    std::string * contents_ptr = reinterpret_cast<std::string*>(user_data);
    std::string & contents_ref = std::reference_wrapper<std::string>(*contents_ptr);
    // Safety precaution
    if (contents_ptr == nullptr)
        return -1;
    // Get the data we need
    char const * const unpacked_data = reinterpret_cast<char *>(p1);
    uint64_t unpacked_size = static_cast<uint64_t>(p2);

    // We're only interested in UCM_PROCESSDATA events
    switch (msg)
    {
    case UCM_PROCESSDATA:
    {
        // Reallocate our buffer and copy the extracted contents
        contents_ptr->reserve(unpacked_size);
        std::copy(&unpacked_data[0], &unpacked_data[unpacked_size], std::back_inserter(contents_ref));
    }
    break;
    default:
        break;
    }

    return 1;
}