#include "archive_7z.hpp"
#include "utf8.hpp"

namespace
{
    static bool const g_initialized = ([]() -> bool
    {
        CrcGenerateTable();
        return true;
    })();
}

namespace mangapp
{
    archive_7z::archive_7z(std::wstring const & filepath)
    {
        // Initialize the 7z alloc structures
        m_alloc = { SzAlloc, SzFree };
        m_tmp_alloc = { SzAllocTemp, SzFreeTemp };
        // Were we able to open the file?
        std::string mbpath = to_utf8(filepath);

        if (InFile_Open(&m_archive_stream.file, mbpath.c_str()) == SZ_OK)
        {
            // Yes, create the vtables for the streams
            FileInStream_CreateVTable(&m_archive_stream);
            LookToRead_CreateVTable(&m_look_stream, False);
            m_look_stream.realStream = &m_archive_stream.s;

            SzArEx_Init(&m_data);
            // Was 7z able to open the archive?
            if (SzArEx_Open(&m_data, &m_look_stream.s, &m_alloc, &m_tmp_alloc) == SZ_OK)
            {
                // Yes, create the entries of the individual archive entries
                for (UInt32 entry_index = 0; entry_index < m_data.NumFiles; entry_index++)
                {
                    m_entries.emplace_back(new entry_7z(&m_data, &m_look_stream.s, &m_alloc, &m_tmp_alloc, entry_index));
                }
            }
            else
            {
                // No, something went wrong - add logging?
            }
        }
        else
        {
            // No, something went wrong - add logging?
        }
    }

    archive_7z::~archive_7z()
    {
        // Cleanup
        File_Close(&m_archive_stream.file);
        SzArEx_Free(&m_data, &m_alloc);
    }
}