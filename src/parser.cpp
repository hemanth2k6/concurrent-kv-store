#include "parser.h"
#include <sstream>

std::vector<std::string> Parser::parse_resp(const std::string &buffer)
{
    std::vector<std::string> args;
    if (buffer.empty() || buffer[0] != '*')
        return args;

    size_t pos = 0;
    size_t line_end = buffer.find("\r\n", pos);

    int num_args = std::stoi(buffer.substr(1, line_end - 1));
    pos = line_end + 2;

    for (int i = 0; i < num_args; ++i)
    {
        if (buffer[pos] == '$')
        {
            line_end = buffer.find("\r\n", pos);
            int str_len = std::stoi(buffer.substr(pos + 1, line_end - pos - 1));
            pos = line_end + 2;

            args.push_back(buffer.substr(pos, str_len));
            pos += str_len + 2;
        }
    }
    return args;
}

std::string Parser::format_resp_bulk(const std::string &str)
{
    if (str.empty())
        return "$-1\r\n";
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}

std::string Parser::format_resp_simple(const std::string &str)
{
    return "+" + str + "\r\n";
}