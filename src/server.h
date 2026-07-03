#pragma once
#include "store.h"
#include <netinet/in.h>

class Server
{
public:
    Server(int port, KVStore &store);
    void start();

private:
    void handle_client(int client_socket);

    int server_fd_;
    struct sockaddr_in address_;
    KVStore &store_;
};