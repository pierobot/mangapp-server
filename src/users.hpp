#ifndef MANGAPP_USERS_HPP
#define MANGAPP_USERS_HPP

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace json11
{
    class Json;
}

namespace mangapp
{
    struct user_data
    {
        std::string m_name;
        std::string m_password;
        std::vector<std::string> m_session_ids;
        std::vector<size_t> m_accessible_paths;
    };

    class users
    {
    public:
        users(json11::Json const & settings_json);
        ~users();

        bool authenticate(std::string const & user, std::string const & password, std::string & session_id);
        bool is_authenticated(std::string const & session_id);
        void logout(std::string const & session_id);

        bool can_access(std::string const & session_id, size_t path_key);
    protected:
    private:
        std::unordered_map<std::string, user_data> m_users;
        std::mutex m_users_mutex;
    };
}

#endif
