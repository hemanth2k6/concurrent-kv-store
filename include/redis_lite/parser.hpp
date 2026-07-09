#pragma once
#include <string>
#include <vector>

class RespParser
{
public:
    std::vector<std::vector<std::string>> feed(const char *data, size_t len);

private:
    std::vector<std::string> tryParse(size_t &consumed);
    std::string buffer_;
};
