#ifndef MANGAPP_ARCHIVE_ZIP_HPP
#define MANGAPP_ARCHIVE_ZIP_HPP

#include "archive.hpp"
#include "archive_zip_entry.hpp"

namespace mangapp
{
    class archive_zip : public archive
    {
    public:
        archive_zip(std::string const & filepath);
        archive_zip(std::wstring const & filepath);

        virtual ~archive_zip();

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

        virtual iterator erase(const_iterator first, const_iterator last) final
        {
            return m_entries.erase(first, last);
        }

        virtual entry_pointer operator[](size_t index) final
        {
            if (m_entries.size() > 1)
                return m_entries[index];
            else
                return nullptr;
        }

        virtual zip_uint64_t count() const final
        {
            return m_entry_count;
        }
    protected:
    private:
        zip_t * m_zip_handle;
        zip_uint64_t m_entry_count;
        std::vector<entry_pointer> m_entries;
    };

}


#endif