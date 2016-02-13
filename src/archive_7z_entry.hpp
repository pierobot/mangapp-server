#ifndef ARCHIVE_7Z_ENTRY_HPP
#define ARCHIVE_7Z_ENTRY_HPP

#include "archive_entry.hpp"

#include <7z/7z.h>
#include <7z/7zCrc.h>
#include <7z/7zFile.h>
#include <7z/7zTypes.h>
#include <7z/7zAlloc.h>

namespace mangapp
{
    /** 
     *  Class that represents a file or directory entry in a 7z archive.
     */
    class entry_7z : public archive_entry
    {
    public:
        /** 
         *  Constructor.
         *  
         *  @param data pointer to the archive data
         *  @param look_stream pointer to ILookInStream
         *  @param alloc pointer to primary ISzAlloc structure
         *  @param tmp_alloc pointer to temporary ISzAlloc structure
         *  @param index index of the entry
         */
        entry_7z(CSzArEx * data, ILookInStream * look_stream, ISzAlloc * alloc, ISzAlloc * tmp_alloc, UInt32 index);
        
        virtual ~entry_7z();

        virtual std::string const contents() const final;

        virtual uint64_t index() const final
        {
            return m_index;
        }

        virtual uint64_t size() const final
        {
            return m_size;
        }

        virtual std::wstring name() const final
        {
            return m_name;
        }

        virtual bool is_dir() const final
        {
            return SzArEx_IsDir(m_data, m_index) != 0;
        }
    protected:
    private:
        CSzArEx * m_data;
        ILookInStream * m_look_stream;
        ISzAlloc * m_alloc;
        ISzAlloc * m_tmp_alloc;
        std::wstring m_name;
        UInt64 m_size;
        UInt32 m_index;
    };

}

#endif