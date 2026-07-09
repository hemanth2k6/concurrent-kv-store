
#pragma once
#include "config.hpp"
#include "store.hpp"
#include "wal.hpp"
#include "thread_pool.hpp"
#include <atomic>

class Server
{
public:
    explicit Server(const Config &config);
    void run();
    void shutdown();

private:
    Config config_;
    Store store_;
    Wal wal_;
    ThreadPool pool_;
    std::atomic<bool> running_{true};
    int listen_fd_ = -1;

    void setupSignalHandlers();
    static void signalHandler(int sig);
    static std::atomic<bool> *instance_running_;
};