#include "http_utility.hpp"

#include <cstdint>

static unsigned char to_hex(unsigned char c)
{
    return  c > 9 ? c + 55 : c + 48;
}

std::string const http_utility::encode_ncr(std::string const & str)
{
    std::string encoded_ncr;
    uint32_t code_point = 0;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (((str[i] >> 7) & 0x0f) == 0x0) // one octet
        {
            // ASCII, no need to change anything
            encoded_ncr.push_back(str[i]);
            continue;
        }
        else if (((str[i] >> 5) & 0x0f) == 0xc) // two octets
        {
            // Get the lower 5 bits of the first octet
            code_point = (str[i++] & 0x1f) << 6;
            // Get the lower 6 bits of the second octet
            code_point |= str[i] & 0x3f;
        }
        else if (((str[i] >> 4) & 0x0f) == 0xe) // three octets
        {
            // Get the lower 4 bits of the first octet
            code_point = (str[i++] & 0x0f) << 12;
            // Get the lower 6 bits of the second and third octet
            code_point |= (str[i++] & 0x3f) << 6;
            code_point |= (str[i] & 0x3f);
        }
        else if (((str[i] >> 3) & 0x0f) == 0xf) // four octets
        {
            // Get the lower 3 bits of the first octet
            code_point = (str[i++] & 0x07) << 18;
            // Get the lower 6 bits of the second, third, and fourth octet
            code_point |= (str[i++] & 0x3f) << 12;
            code_point |= (str[i++] & 0x3f) << 6;
            code_point |= (str[i] & 0x3f);
        }

        encoded_ncr.push_back('&');
        encoded_ncr.push_back('#');
        encoded_ncr += std::to_string(code_point);
        encoded_ncr.push_back(';');
    }

    return encoded_ncr;
}

std::string const http_utility::encode_uri(std::string const & uri)
{
    std::string encoded_uri;

    for (unsigned char c : uri)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '~' || c == '.')
        {
            encoded_uri.push_back(c);
        }
        else
        {
            encoded_uri.push_back('%');
            encoded_uri.push_back(to_hex(c >> 4)); // Upper 4 bits
            encoded_uri.push_back(to_hex(c & 0xf)); // Lower 4 bits
        }
    }

    return encoded_uri;
}