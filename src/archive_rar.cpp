#include "archive_rar.hpp"

namespace mangapp
{
    archive_rar::archive_rar(std::wstring const & filepath)
    {
        RARHeaderDataEx rar_header = { 0 };
        RAROpenArchiveDataEx arc_data = { 0 };
        arc_data.ArcNameW = const_cast<wchar_t *>(&filepath[0]);
        arc_data.OpenMode = RAR_OM_LIST;

        m_rar_handle = RAROpenArchiveEx(&arc_data);
        // Were we able to open the RAR archive?
        if (m_rar_handle != nullptr)
        {
            // Yes, iterate through the file or directory entries
            for (uint64_t entry_index = 0; RARReadHeaderEx(m_rar_handle, &rar_header) == ERAR_SUCCESS; ++entry_index)
            {
                // Test the flags to determine if it's a directory
                bool is_dir = ((arc_data.Flags & RHDF_DIRECTORY) == RHDF_DIRECTORY);
                // Create the rar entry and emplace it in our container
                m_entries.emplace_back(new entry_rar(filepath, &rar_header.FileNameW[0], is_dir, rar_header.UnpSize, entry_index));
                // Get the next entry
                RARProcessFile(m_rar_handle, RAR_SKIP, NULL, NULL);
            }

            // Cleanup
            RARCloseArchive(m_rar_handle);
        }
        else
        {
            // No, something went wrong - add logging?
        }
    }

    archive_rar::~archive_rar()
    {
    }
}
