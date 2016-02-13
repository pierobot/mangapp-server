#include "directory_file_entry.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace base;

file_entry::file_entry(std::wstring const & path, std::wstring const & name, size_t key) :
    m_path(path),
    m_name(name),
    m_key(key),
    m_size(boost::filesystem::file_size(path + name))
{
}

file_entry::~file_entry()
{
}

std::string file_entry::contents() const
{
    std::string contents;
    
    using namespace boost::filesystem;
    ifstream fstream(path(m_path + m_name), std::ios::in | std::ios::binary);
    
    if (fstream.good() == true)
    {
        // Reallocate our buffer to the appropriate size and read the file's contents
        contents.resize(m_size);
        fstream.read(&contents[0], m_size);
    }

    return contents;
}


std::wstring const & file_entry::get_path() const
{
    return m_path;
}

std::wstring const & file_entry::get_name() const
{
    return m_name;
}

std::wstring const file_entry::get_extension() const
{
    std::wstring extension;
    std::wstring file_path = m_path + m_name;

    auto ext_start_pos = file_path.rfind(L'.');
    if (ext_start_pos != std::string::npos)
    {
        auto ext_end_pos = file_path.length();
        extension = file_path.substr(ext_start_pos, ext_end_pos - ext_start_pos);
    }

    return extension;
}

size_t const file_entry::get_key() const
{
    return m_key;
}