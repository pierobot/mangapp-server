#ifndef ARCHIVE_RAR_ENTRY_HPP
#define ARCHIVE_RAR_ENTRY_HPP

#include "archive_entry.hpp"

#include <unrar/rar.hpp>
#include <unrar/dll.hpp>

namespace mangapp
{
    /**
    *  Class that represents a file or directory entry in a RAR archive.
    */
    class entry_rar : public archive_entry
    {
    public:
        /** 
         *  Constructor.
         *
         *  @param rar_path full file path of the rar archive
         *  @param name name of the entry
         *  @param is_dir flag that indicates whether the entry is a directory or not
         *  @param size the size of the entry in bytes
         *  @param inde the index of the entry
         */
        entry_rar(std::wstring rar_path, std::wstring name, bool is_dir, uint64_t size, uint64_t index);

        virtual ~entry_rar();

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

        virtual bool is_dir() const final
        {
            return m_is_dir;
        }

        /** 
          * Callback that will be called by the unrar library.
          * Technically, it's possible that there could be multiple calls to this callback if the
            uncompressed data is given to us in chunks.
            This means that our current implementation is, *cough*, terrible.
            What to do? Write an algorithm that can handle multiple chunks of uncompressed data.
            Will implement later on.
          
          * @param msg the type of event message
          * @param p1 library supplied data
          * @param p2 library supplied data
          * @return 1 if successful; -1 if something went wrong
         */
        static int CALLBACK rar_callback(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2);
    protected:
    private:
        std::wstring m_rar_path;
        std::wstring m_name;
        uint64_t m_size;
        uint64_t m_index;
        bool m_is_dir;
    };
}

#endif
