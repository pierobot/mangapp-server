#include "archive_zip_entry.hpp"
#include "utf8.hpp"

#include <cstring>

using namespace mangapp;

entry_zip::entry_zip(zip_t * const zip_handle, zip_uint64_t index) :
    m_zip_handle(zip_handle),
    m_index(index),
    m_zip_error(zip_stat_index(zip_handle, index, 0, &m_zip_stat)),
    m_name()
{
    if (m_zip_stat.valid & ZIP_STAT_NAME)
    {
        auto length = std::strlen(&m_zip_stat.name[0]);
        m_name = to_utf16(std::string(&m_zip_stat.name[0], &m_zip_stat.name[length]));

        wchar_t trailing_char = m_name[m_name.length() - 1];
        m_is_directory = (trailing_char == L'\\' || trailing_char == L'/');
    }
}

entry_zip::~entry_zip()
{
}

std::string const entry_zip::contents() const
{
    std::string file_contents;
    zip_uint64_t file_size = size();
    file_contents.resize(file_size);

    if (m_zip_handle != nullptr && file_size != 0)
    {
        zip_file_t * file_handle = zip_fopen_index(m_zip_handle, m_index, 0);

        if (file_handle != nullptr)
        {
            zip_fread(file_handle, &file_contents[0], file_contents.size());
            zip_fclose(file_handle);
        }
    }

    return file_contents;
}
