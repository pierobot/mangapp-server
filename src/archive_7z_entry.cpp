#include "archive_7z_entry.hpp"

#include <iterator>

namespace mangapp
{
    entry_7z::entry_7z(CSzArEx * data,
                       ILookInStream * look_stream,
                       ISzAlloc * alloc,
                       ISzAlloc * tmp_alloc,
                       UInt32 index) :
        m_data(data),
        m_look_stream(look_stream),
        m_alloc(alloc),
        m_tmp_alloc(tmp_alloc),
        m_index(index),
        m_size(SzArEx_GetFileSize(data, index))
    {
        // Get the UTF16 string of the file
        size_t name_length = SzArEx_GetFileNameUtf16(m_data, m_index, nullptr);
        m_name.resize(name_length);
        SzArEx_GetFileNameUtf16(m_data, m_index, reinterpret_cast<UInt16 *>(&m_name[0]));
    }

    entry_7z::~entry_7z()
    {
    }

    std::string const entry_7z::contents() const
    {
        std::string contents_7z;

        UInt32 block_index = 0xfffffff;
        size_t offset = 0;
        size_t processed = 0;
        size_t out_size = 0;
        Byte * out_buffer = nullptr;

        // Were we able to extract the contents of this entry?
        if (SZ_OK == SzArEx_Extract(m_data, m_look_stream, m_index, &block_index,
            &out_buffer, &out_size, &offset, &processed, m_alloc, m_tmp_alloc))
        {
            // Yes, copy the contents of the extracted buffer to our string
            std::copy(&out_buffer[offset], &out_buffer[offset + m_size], std::back_inserter(contents_7z));
            // Cleanup
            IAlloc_Free(m_alloc, out_buffer);
        }

        return contents_7z;
    }
}
