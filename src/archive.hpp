#ifndef MANGAPP_ARCHIVE_HPP
#define MANGAPP_ARCHIVE_HPP

#include "archive_entry.hpp"

#include <memory>

namespace mangapp
{
    /** 
     *  Basic interface for compressed archives.
     */
    class archive
    {
    public:
        typedef std::shared_ptr<archive_entry> entry_pointer;
        typedef std::vector<entry_pointer>::iterator iterator;
        typedef std::vector<entry_pointer>::const_iterator const_iterator;

        virtual iterator begin() = 0;
        virtual iterator end() = 0;
        virtual const_iterator cbegin() const = 0;
        virtual const_iterator cend() const = 0;
        virtual iterator erase(const_iterator first, const_iterator last) = 0;

        virtual uint64_t count() const = 0;

        virtual entry_pointer operator[](size_t index) = 0;

        static std::unique_ptr<archive> open(std::wstring const & file_path);
    protected:
    private:
    };
}

#endif