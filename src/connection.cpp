
#include "redis_lite/connection.hpp"
#include "redis_lite/logger.hpp"
#include <unistd.h>
#include <errno.h>

Connection::Connection(int client_fd, Store &store, Wal &wal)
    : fd_(client_fd), store_(store), wal_(wal) {}

void Connection::run()
{
    constexpr size_t BUF_SIZE = 4096;
    char buf[BUF_SIZE];
    while (true)
    {
        ssize_t n = read(fd_, buf, sizeof(buf));
        if (n <= 0)
        {
            if (n == 0)
                break; // EOF
            if (errno == EINTR)
                continue;
            LOG_ERROR("Read error on fd " + std::to_string(fd_));
            break;
        }
        auto commands = parser_.feed(buf, n);
        for (auto &cmd : commands)
        {
            std::string resp = dispatch(cmd);
            if (!writeResponse(resp))
            {
                return;
            }
        }
    }
}

bool Connection::writeResponse(const std::string &resp)
{
    writeBuffer_ += resp;
    const char *ptr = writeBuffer_.data();
    size_t remaining = writeBuffer_.size();
    while (remaining > 0)
    {
        ssize_t sent = write(fd_, ptr, remaining);
        if (sent <= 0)
        {
            if (errno == EINTR)
                continue;
            return false;
        }
        remaining -= sent;
        ptr += sent;
    }
    writeBuffer_.clear();
    return true;
}

std::string Connection::dispatch(const std::vector<std::string> &args)
{
    if (args.empty())
        return "-ERR empty command\r\n";
    const std::string &cmd = args[0];

    if (cmd == "PING")
    {
        return "+PONG\r\n";
    }
    else if (cmd == "SET")
    {
        if (args.size() != 3)
            return "-ERR wrong number of arguments for 'SET'\r\n";
        // Reconstruct RESP for WAL
        std::string resp_cmd = "*3\r\n$3\r\nSET\r\n$" + std::to_string(args[1].size()) + "\r\n" +
                               args[1] + "\r\n$" + std::to_string(args[2].size()) + "\r\n" + args[2] + "\r\n";
        wal_.append(resp_cmd);
        store_.set(args[1], args[2]);
        return "+OK\r\n";
    }
    else if (cmd == "GET")
    {
        if (args.size() != 2)
            return "-ERR wrong number of arguments for 'GET'\r\n";
        auto val = store_.get(args[1]);
        if (val)
        {
            return "$" + std::to_string(val->size()) + "\r\n" + *val + "\r\n";
        }
        else
        {
            return "$-1\r\n";
        }
    }
    else if (cmd == "DEL")
    {
        if (args.size() < 2)
            return "-ERR wrong number of arguments for 'DEL'\r\n";
        int deleted = 0;
        for (size_t i = 1; i < args.size(); ++i)
        {
            if (store_.del(args[i]))
                deleted++;
        }
        std::string resp_cmd = "*" + std::to_string(args.size()) + "\r\n";
        for (const auto &a : args)
        {
            resp_cmd += "$" + std::to_string(a.size()) + "\r\n" + a + "\r\n";
        }
        wal_.append(resp_cmd);
        return ":" + std::to_string(deleted) + "\r\n";
    }
    else
    {
        return "-ERR unknown command '" + cmd + "'\r\n";
    }
}