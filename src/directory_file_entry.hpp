#ifndef DIRECTORY_FILE_ENTRY_HPP
#define DIRECTORY_FILE_ENTRY_HPP

#include <string>

namespace base
{
    class file_entry
    {
    public:
        file_entry(std::wstring const & path, std::wstring const & name, size_t key);

        virtual ~file_entry();

        std::string contents() const;
        std::wstring const & get_path() const;
        std::wstring const & get_name() const;
        std::wstring const   get_extension() const;
        size_t const get_key() const;
    protected:
    private:
        std::wstring const m_path;
        std::wstring const m_name;
        size_t const m_key;
        size_t const m_size;
    };
}

#endif