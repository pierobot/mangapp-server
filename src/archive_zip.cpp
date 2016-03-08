#include "archive_zip.hpp"
#include "utf8.hpp"

#include <algorithm>

namespace mangapp
{
    archive_zip::archive_zip(std::string const & filepath)
    {
        int error = 0;
        m_zip_handle = zip_open(filepath.c_str(), ZIP_RDONLY, &error);

        if (m_zip_handle != nullptr && error == ZIP_ER_OK)
        {
            m_entry_count = zip_get_num_entries(m_zip_handle, 0);

            for (zip_uint64_t entry_index = 0; entry_index < m_entry_count; entry_index++)
            {
                m_entries.emplace_back(new entry_zip(m_zip_handle, entry_index));
            }

            // Erase any directories
            m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
                [this](entry_pointer const & value) -> bool
            {
                return value->is_dir();
            }), m_entries.end());
        }
    }

    /**
    *   Constructor to fulfill the is_constructible requirement of archive::open.
    *   Additionally, libzip does not provide a function to open files from a wchar_t* path, but it does
    *   internally convert a UTF-8 path to UTF-16 on Windows; which is why we must convert our wstring to UTF-8.
    */
    archive_zip::archive_zip(std::wstring const & filepath) :
        archive_zip(to_utf8(filepath))
    {

    }

    archive_zip::~archive_zip()
    {
        if (m_zip_handle != nullptr)
            zip_close(m_zip_handle);
    }
}
