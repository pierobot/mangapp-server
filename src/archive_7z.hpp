#ifndef MANGAPP_7Z_ARCHIVE_HPP
#define MANGAPP_7Z_ARCHIVE_HPP

#include "archive.hpp"
#include "archive_7z_entry.hpp"

namespace mangapp
{
    /** 
     *  Class that represents a 7z archive.
     */
    class archive_7z : public archive
    {
    public:
        /** 
         *  Constructor... not much else to say.
         *  
         *  @param filepath the filepath of the 7z archive
         */
        archive_7z(std::wstring const & filepath);

        virtual ~archive_7z();

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
        CFileInStream m_archive_stream;
        CLookToRead m_look_stream;
        ISzAlloc m_alloc;
        ISzAlloc m_tmp_alloc;
        CSzArEx m_data;
        std::vector<entry_pointer> m_entries;
    };
}

#endif