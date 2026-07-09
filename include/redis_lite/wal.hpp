#pragma once
#include <string>
#include <mutex>
#include <vector>
#include "store.hpp"

class Wal
{
public:
    explicit Wal(const std::string &filepath);
    ~Wal();
    void append(const std::string &raw_resp);
    void replayInto(Store &store);

private:
    int fd_;
    std::mutex mutex_;
    std::string path_;

    static std::vector<std::string> parseCommand(const std::string &data);
    static std::string buildResp(const std::vector<std::string> &args);
};
