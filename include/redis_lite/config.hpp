#pragma once
#include <string>
struct Config
{
    int port = 6379;
    std::string wal_file = "wal.log";
    size_t thread_pool_size = 4;
    bool daemonize = false;
};
