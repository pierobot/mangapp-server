#ifndef MANGAPP_ARCHIVE_ENTRY_HPP
#define MANGAPP_ARCHIVE_ENTRY_HPP

#include <stdint.h>
#include <string>
#include <vector>

namespace mangapp
{
    /** 
     *  Basic interface for a file or directory entry in a compressed archive.
     */
    class archive_entry
    {
    public:
        virtual ~archive_entry() {}

        virtual std::string const contents() const = 0;
        virtual uint64_t index() const = 0;
        virtual uint64_t size() const = 0;
        virtual std::wstring name() const = 0;
        virtual std::wstring extension() const = 0;
        virtual bool is_dir() const = 0;
    protected:
    private:
    };
}

#endif