#ifndef ARCHIVE_ZIP_ENTRY_HPP
#define ARCHIVE_ZIP_ENTRY_HPP

#include "archive_entry.hpp"

#include <locale>

#include <zip.h>

namespace mangapp
{
    class entry_zip : public archive_entry
    {
    public:
        entry_zip(zip_t * const zip_handle, zip_uint64_t index);

        virtual ~entry_zip();


        virtual std::string const contents() const final;

        int last_error() const
        {
            return m_zip_error;
        }

        virtual zip_uint64_t index() const final
        {
            return m_index;
        }

        virtual std::wstring name() const final
        {
            return m_name;
        }

        virtual std::wstring extension() const final
        {
            std::wstring extension_str;

            auto extension_start_pos = m_name.rfind(L'.');
            if (extension_start_pos != std::string::npos)
            {
                extension_str = m_name.substr(extension_start_pos);
            }

            return extension_str;
        }

        virtual zip_uint64_t size() const final
        {
            return m_zip_stat.valid & ZIP_STAT_SIZE ? m_zip_stat.size : 0;
        }

        virtual bool is_dir() const final
        {
            return m_is_directory;
        }
    protected:
    private:
        zip_t * const m_zip_handle;
        zip_uint64_t m_index;
        zip_stat_t m_zip_stat;
        int m_zip_error;
        std::wstring m_name;
        bool m_is_directory;
    };
}

#endif
