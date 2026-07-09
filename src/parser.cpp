#include "redis_lite/parser.hpp"
#include <stdexcept>

std::vector<std::vector<std::string>> RespParser::feed(const char *data, size_t len)
{
    buffer_.append(data, len);
    std::vector<std::vector<std::string>> commands;
    while (true)
    {
        size_t consumed = 0;
        auto cmd = tryParse(consumed);
        if (!cmd.empty())
        {
            commands.push_back(std::move(cmd));
            buffer_.erase(0, consumed);
        }
        else
        {
            break;
        }
    }
    return commands;
}

std::vector<std::string> RespParser::tryParse(size_t &consumed)
{
    if (buffer_.empty() || buffer_[0] != '*')
    {
        consumed = 0;
        return {};
    }
    size_t pos = 0;
    size_t header_end = buffer_.find("\r\n", pos);
    if (header_end == std::string::npos)
    {
        consumed = 0;
        return {};
    }
    std::string count_str = buffer_.substr(1, header_end - 1);
    int count = 0;
    try
    {
        count = std::stoi(count_str);
    }
    catch (...)
    {
        consumed = 1;
        return {};
    }
    pos = header_end + 2;
    std::vector<std::string> tokens;
    tokens.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        if (pos >= buffer_.size() || buffer_[pos] != '$')
        {
            consumed = 0;
            return {};
        }
        size_t bulk_header_end = buffer_.find("\r\n", pos);
        if (bulk_header_end == std::string::npos)
        {
            consumed = 0;
            return {};
        }
        std::string len_str = buffer_.substr(pos + 1, bulk_header_end - (pos + 1));
        int blen = 0;
        try
        {
            blen = std::stoi(len_str);
        }
        catch (...)
        {
            consumed = pos;
            return {};
        }
        pos = bulk_header_end + 2;
        if (blen == -1)
        {
            tokens.push_back("");
        }
        else
        {
            if (pos + blen + 2 > buffer_.size())
            {
                consumed = 0;
                return {};
            }
            tokens.push_back(buffer_.substr(pos, blen));
            pos += blen + 2;
        }
    }
    consumed = pos;
    return tokens;
}