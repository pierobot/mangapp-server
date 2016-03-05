#include "archive.hpp"

#include "archive_zip.hpp"
#include "archive_rar.hpp"
#include "archive_7z.hpp"

namespace mangapp
{
    std::unique_ptr<archive> archive::open(std::wstring const & file_path)
    {
        auto extension_start_pos = file_path.rfind(L'.');
        std::wstring extension(file_path.substr(extension_start_pos));

        if (extension == L".zip" || extension == L".cbz")
        {
            return std::make_unique<archive_zip>(file_path);
        }
        else if (extension == L".rar" || extension == L".cbr")
        {
            return std::make_unique<archive_rar>(file_path);
        }
        else if (extension == L".7z" || extension == L".cb7")
        {
            return std::make_unique<archive_7z>(file_path);
        }

        return nullptr;
    }
}
