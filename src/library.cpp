#include "library.hpp"

#include "natsort.hpp"

#include <iostream>
#include <set>







//json11::Json const library::get_list() const
//{
//    std::vector<std::reference_wrapper<json11::Json const>> entries;
//
//    std::for_each(m_entries.cbegin(), m_entries.cend(),
//        [&entries](library_map_t::value_type const & entry)
//    {
//        entries.push_back(entry.second->to_json());
//    });
//
//    entries = entries;
//
//    std::sort(entries.begin(), entries.end(),
//        [](json11::Json const & left, json11::Json const & right) -> bool
//    {
//        auto left_str = left.array_items()[1].string_value();
//        auto right_str = right.array_items()[1].string_value();
//
//        return natural::sort_less<char>()(left_str, right_str);
//    });
//    
//    return json11::Json(entries);
//}

//std::string library::read_file(size_t key, std::wstring const & file_path) const
//{
//    using namespace boost::filesystem;
//
//    std::string file_data;
//
//    if (exists(file_path) == true)
//    {
//        auto size = file_size(file_path);
//        ifstream fstream(path(file_path), std::ios::in | std::ios::binary);
//
//        if (fstream.good() == true)
//        {
//            // Reallocate our buffer to the appropriate size and read the file's contents
//            file_data.resize(size);
//            fstream.read(&file_data[0], size);
//
//            return file_data;
//        }
//    }
//
//    return std::string("");
//}

//std::string library::get_thumbnail(size_t key, bool folder, uint32_t index) const
//{
//    std::string thumb_data;
//    
//    auto entry_iterator = m_entries.find(key);
//    // Does the specified key exist in our map of manga?
//    if (entry_iterator != m_entries.cend())
//    {
//        // Yes, do we want the folder's thumbnail? 
//        if (folder == true)
//        {
//            // Yes, get the contents of "folder.jpg"
//            std::wstring folder_thumb_path = entry_iterator->second->get_path() + 
//                                             entry_iterator->second->get_name() + L"/folder.jpg";
//            thumb_data = read_file(key, folder_thumb_path);
//        }
//        else
//        {
//            // No, get the thumbnail from an archive
//            // NOT IMPLEMENTED YET
//        }
//    }
//
//    return thumb_data;
//}
//
//void library::search_mangaupdates_for(std::string const & name, std::function<void(json11::Json)> on_event)
//{
//    std::string search_url = "/series.html?page=1&stype=title&search=" + http_utility::encode_uri(http_utility::encode_ncr(name));
//
//    auto on_error = [on_event](std::string const & error_msg)
//    {
//        on_event(json11::Json({ "Result", "Failure: " + error_msg }));
//    };
//
//    m_http_helper.http_get_async("www.mangaupdates.com", search_url,
//        [this, name, on_event, on_error](std::string const & contents)
//    {
//        std::string series_id = mangaupdates::get_id(contents, name);
//        if (series_id == "")
//            on_error(std::string("Unable to find manga with name: ") + name);
//
//        std::string series_url = "/series.html?id=" + series_id;
//
//        m_http_helper.http_get_async("www.mangaupdates.com", series_url,
//            [series_id, on_event](std::string const & contents)
//        {
//            auto description = mangaupdates::get_description(contents);
//            auto associated_names = mangaupdates::get_associated_names(contents);
//            auto genres = mangaupdates::get_genres(contents);
//            auto authors = mangaupdates::get_authors(contents);
//            auto artists = mangaupdates::get_artists(contents);
//            auto year = mangaupdates::get_year(contents);
//
//            on_event(json11::Json::object{
//                { "Result", "Success" },
//                { "Id", series_id },
//                { "AssociatedNames", associated_names },
//                { "Genres", genres },
//                { "Authors", authors },
//                { "Artists", artists },
//                { "Year", year } });
//        }, on_error);
//    }, on_error);
//}
//
// void library::get_manga_details(size_t key, std::function<void(json11::Json)> on_event)
// {
//    // Is the key valid?
//    auto manga_iterator = m_entries.find(key);
//    if (manga_iterator != m_entries.cend())
//    {
//        // Yes, proceed
//        std::wstring const & wname = manga_iterator->second->get_name();
//        std::string mbname;
//        
//        // Encode the name to UTF-8
//        utf8::utf16to8(wname.cbegin(), wname.cend(), std::back_inserter(mbname));
//        
//        search_mangaupdates_for(mbname, on_event);
//    }
//}
//
// json11::Json library::enumerate_files(size_t key) const
// {
//     std::vector<std::string> files;
//
//     auto manga_iterator = m_entries.find(key);
//     if (manga_iterator != m_entries.cend())
//     {
//         std::wstring wpath = manga_iterator->second->get_path() + manga_iterator->second->get_name();
//         
//         using namespace boost::filesystem;
//         // Ensure the path exists and that it's a directory
//         if (exists(wpath) == true && is_directory(wpath) == true)
//         {
//             // Iterate through the directories elements
//             for (auto const & entry : boost::make_iterator_range(directory_iterator(wpath), directory_iterator()))
//             {
//                 // We only want files that have the appropriate extension
//                 if (is_in_container(g_archive_extensions, entry.path().extension()) == true)
//                 {
//                     // Encode to UTF-8 and add it to our vector
//                     std::wstring wpath(entry.path().filename().generic_wstring());
//                     std::string mbpath;
//                     utf8::utf16to8(wpath.cbegin(), wpath.cend(), std::back_inserter(mbpath));
//
//                     files.push_back(mbpath);
//                 }
//             }
//
//             std::sort(files.begin(), files.end(), natural::sort_less<char>());
//         }
//     }
//
//     return json11::Json(files);
// }
//