#pragma once
#include "parser.hpp"
#include "store.hpp"
#include "wal.hpp"
#include <string>

class Connection
{
public:
    Connection(int client_fd, Store &store, Wal &wal);
    void run();

private:
    std::string dispatch(const std::vector<std::string> &args);
    bool writeResponse(const std::string &resp);
    int fd_;
    Store &store_;
    Wal &wal_;
    RespParser parser_;
    std::string writeBuffer_;
};