
#include "redis_lite/server.hpp"
#include "redis_lite/connection.hpp"
#include "redis_lite/logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <csignal>

std::atomic<bool> *Server::instance_running_ = nullptr;

Server::Server(const Config &config)
    : config_(config),
      wal_(config.wal_file),
      pool_(config.thread_pool_size)
{

    wal_.replayInto(store_);
}

void Server::signalHandler(int /*sig*/)
{
    if (instance_running_)
        instance_running_->store(false);
}

void Server::setupSignalHandlers()
{
    instance_running_ = &running_;
    struct sigaction sa{};
    sa.sa_handler = &Server::signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    signal(SIGPIPE, SIG_IGN);
}

void Server::run()
{
    setupSignalHandlers();

    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ == -1)
    {
        LOG_ERROR("socket() failed: " + std::string(strerror(errno)));
        return;
    }
    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(config_.port);

    if (bind(listen_fd_, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        LOG_ERROR("bind() failed: " + std::string(strerror(errno)));
        close(listen_fd_);
        return;
    }
    if (listen(listen_fd_, SOMAXCONN) < 0)
    {
        LOG_ERROR("listen() failed: " + std::string(strerror(errno)));
        close(listen_fd_);
        return;
    }

    LOG_INFO("Server listening on port " + std::to_string(config_.port));

    while (running_)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd_, (sockaddr *)&client_addr, &client_len);
        if (client_fd < 0)
        {
            if (errno == EINTR)
                continue;
            if (!running_)
                break;
            LOG_ERROR("accept() failed: " + std::string(strerror(errno)));
            break;
        }
        LOG_INFO("New connection from " + std::string(inet_ntoa(client_addr.sin_addr)));

        // Capture by copy; Connection will own the fd
        pool_.enqueue([this, client_fd]
                      {
            Connection conn(client_fd, store_, wal_);
            conn.run();
            close(client_fd); });
    }

    LOG_INFO("Shutting down...");
    pool_.shutdown();
    if (listen_fd_ != -1)
        close(listen_fd_);
}

void Server::shutdown()
{
    running_ = false;
}