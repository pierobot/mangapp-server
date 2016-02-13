#ifndef MANGAPP_ARCHIVE_RAR_HPP
#define MANGAPP_ARCHIVE_RAR_HPP

#include "archive.hpp"
#include "archive_rar_entry.hpp"

namespace mangapp
{
    class archive_rar : public archive
    {
    public:
        archive_rar(std::wstring const & filepath);

        virtual ~archive_rar();

        virtual iterator begin() final
        {
            return m_entries.begin();
        }

        virtual iterator end() final
        {
            return m_entries.end();
        }
        
        virtual const_iterator cbegin() const final
        {
            return m_entries.cbegin();
        }

        virtual const_iterator cend() const final
        {
            return m_entries.cend();
        }

        virtual entry_pointer operator[](size_t index) final
        {
            return m_entries[index];
        }

        virtual uint64_t count() const final
        {
            return m_entries.size();
        }

    protected:
    private:
        HANDLE m_rar_handle;
        std::vector<entry_pointer> m_entries;
    };
}

#endif