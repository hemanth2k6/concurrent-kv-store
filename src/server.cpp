#include "server.h"
#include "parser.h"
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <iostream>

Server::Server(int port, KVStore &store) : store_(store)
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port);

    bind(server_fd_, (struct sockaddr *)&address_, sizeof(address_));
}

void Server::start()
{
    listen(server_fd_, 10);
    std::cout << "Redis-compatible Server listening on port 6379...\n";

    while (true)
    {
        int addrlen = sizeof(address_);
        int client_socket = accept(server_fd_, (struct sockaddr *)&address_, (socklen_t *)&addrlen);

        std::thread(&Server::handle_client, this, client_socket).detach();
    }
}

void Server::handle_client(int client_socket)
{
    char buffer[1024] = {0};

    while (true)
    {
        int valread = read(client_socket, buffer, 1024);
        if (valread <= 0)
            break;

        std::vector<std::string> args = Parser::parse_resp(std::string(buffer, valread));
        if (args.empty())
            continue;

        std::string command = args[0];
        for (auto &c : command)
            c = toupper(c);

        std::string response;

        if (command == "SET" && args.size() >= 3)
        {
            store_.set(args[1], args[2]);
            response = Parser::format_resp_simple("OK");
        }
        else if (command == "GET" && args.size() >= 2)
        {
            std::string val = store_.get(args[1]);
            response = Parser::format_resp_bulk(val);
        }
        else
        {
            response = "-ERR unknown command\r\n";
        }

        send(client_socket, response.c_str(), response.length(), 0);
    }
    close(client_socket);
}