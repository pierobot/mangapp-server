#include "users.hpp"
#include "utf8.hpp"

#include <algorithm>

#include <json11/json11.hpp>

#include <boost/functional/hash.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace
{
    std::string const generate_session_id()
    {
        std::string session_id;
        std::string characters("abcdefghijklmnopqrstuvwxyz"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "1234567890");

        session_id.resize(64);
        boost::random::random_device rng;
        boost::random::uniform_int_distribution<> distribution(0, characters.size() - 1);

        std::generate(session_id.begin(), session_id.end(),
            [&]() -> char
        {
            return characters[distribution(rng)];
        });

        return session_id;
    }
}

namespace mangapp
{
    users::users(json11::Json const & settings_json) :
        m_users_mutex()
    {
        for (auto const & users : settings_json["users"].array_items())
        {
            json11::Json const & object = users.object_items();
            auto const & name = object["name"].string_value();
            auto const & password = object["password"].string_value();
            user_data data = { name, password, {} };

            m_users.emplace(name, data);
        }

        for (auto const & paths : settings_json["manga"].array_items())
        {
            json11::Json const & object = paths.object_items();
            auto const path = to_utf16(object["path"].string_value());
            
            for (auto const & user : object["access"].array_items())
            {
                auto user_iterator = m_users.find(user.string_value());
                if (user_iterator != m_users.cend())
                {
                    size_t path_hash = boost::hash<std::wstring>()(path);
                    m_users[user.string_value()].m_accessible_paths.push_back(path_hash);
                }
            }
        }
    }

    users::~users()
    {
    }

    bool users::authenticate(std::string const & user, std::string const & password, std::string & session_id)
    {
        bool authenticated = false;

        std::lock_guard<std::mutex> lock(m_users_mutex);
        auto search_iterator = std::find_if(m_users.begin(), m_users.end(),
            [&user, &password](decltype(m_users)::value_type & value) -> bool
        {
            return value.second.m_name == user && value.second.m_password == password;
        });

        if (search_iterator != m_users.cend())
        {
            session_id = generate_session_id();
            search_iterator->second.m_session_ids.push_back(session_id);
            authenticated = true;
        }

        return authenticated;
    }

    bool users::is_authenticated(std::string const & session_id)
    {
        std::lock_guard<std::mutex> lock(m_users_mutex);
        auto search_iterator = std::find_if(m_users.cbegin(), m_users.cend(),
            [this, &session_id](decltype(m_users)::value_type const & value) -> bool
        {
            auto const & session_ids = value.second.m_session_ids;
            return std::find(session_ids.cbegin(), session_ids.cend(), session_id) != session_ids.cend();
        });

        return search_iterator != m_users.cend();
    }

    void users::logout(std::string const & session_id)
    {
        std::lock_guard<std::mutex> lock(m_users_mutex);
        for (auto & user : m_users)
        {
            auto & session_ids = user.second.m_session_ids;
            session_ids.erase(std::remove(session_ids.begin(), session_ids.end(), session_id), session_ids.end());
        }
    }

    bool users::can_access(std::string const & session_id, size_t path_key)
    {
        bool has_access = false;

        std::lock_guard<std::mutex> lock(m_users_mutex);
        auto search_iterator = std::find_if(m_users.cbegin(), m_users.cend(),
            [this, &session_id](decltype(m_users)::value_type const & value) -> bool
        {
            auto const & session_ids = value.second.m_session_ids;
            return std::find(session_ids.cbegin(), session_ids.cend(), session_id) != session_ids.cend();
        });

        if (search_iterator != m_users.cend())
        {
            auto const & accessible_paths = search_iterator->second.m_accessible_paths;
            has_access = std::find(accessible_paths.cbegin(), accessible_paths.cend(), path_key) != accessible_paths.cend();
        }

        return has_access;
    }
}