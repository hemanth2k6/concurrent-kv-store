#include "redis_lite/wal.hpp"
#include "redis_lite/logger.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
Wal::Wal(const std::string &filepath) : path_(filepath)
{
    Store temp;
    fd_ = open(filepath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_ == -1)
    {
        throw std::runtime_error("cannot open wal file: " + std::string(strerror(errno)));
    }
}

Wal::~Wal()
{
    if (fd_ != -1)
        close(fd_);
}

void Wal::append(const std::string &raw_resp)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t len = raw_resp.size();
    if (write(fd_, &len, sizeof(len)) != sizeof(len))
    {
        LOG_ERROR("wal write failed (length)");
        return;
    }
    if (write(fd_, raw_resp.data(), raw_resp.size()) != static_cast<ssize_t>(raw_resp.size()))
    {
        LOG_ERROR("wal write failed(data)");
        return;
    }
    fsync(fd_);
}

void Wal::replayInto(Store &store)
{
    int rfd = open(path_.c_str(), O_RDONLY);
    if (rfd == -1)
    {
        if (errno == ENOENT)
        {
            return;
        }
        LOG_ERROR("wal replay open failed: " + std::string(strerror(errno)));
        return;
    }
    LOG_INFO("replaying wal...");
    while (true)
    {
        uint32_t len;
        ssize_t n = read(rfd, &len, sizeof(len));
        if (n == 0)
            break;
        if (n != sizeof(len))
        {
            LOG_ERROR("Corrupted wal");
            break;
        }
        std::string raw(len, '\0');
        n = read(rfd, &raw[0], len);
        if (n != len)
        {
            LOG_ERROR("Corrupted wal");
            break;
        }
        auto args = parseCommand(raw);
        if (args.empty())
            continue;
        if (args[0] == "SET" && args.size() == 3)
        {
            store.set(args[1], args[2]);
        }
        else if (args[0] == "DEL" && args.size() >= 2)
        {
            for (size_t i = 1; i < args.size(); ++i)
            {
                store.del(args[i]);
            }
        }
    }
    close(rfd);
    LOG_INFO("Wal replay complete");
}

std::vector<std::string> Wal::parseCommand(const std::string &data)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    if (pos >= data.size() || data[pos] != '*')
        return tokens;
    pos++;
    size_t end = data.find("\r\n", pos);
    if (end == std::string::npos)
        return tokens;
    int count = std::stoi(data.substr(pos, end - pos));
    pos = end + 2;
    for (int i = 0; i < count; ++i)
    {
        if (pos >= data.size() || data[pos] != '$')
            return tokens;
        pos++;
        end = data.find("\r\n", pos);
        if (end == std::string::npos)
            return tokens;
        int blen = std::stoi(data.substr(pos, end - pos));
        pos = end + 2;
        if (pos + blen > data.size())
            return tokens;
        tokens.push_back(data.substr(pos, blen));
        pos += blen + 2;
    }
    return tokens;
}