#pragma once
#include <string>
#include <vector>

class Parser
{
public:
    static std::vector<std::string> parse_resp(const std::string &buffer);
    static std::string format_resp_bulk(const std::string &str);
    static std::string format_resp_simple(const std::string &str);
};