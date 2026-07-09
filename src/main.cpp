#include "redis_lite/server.hpp"
#include "redis_lite/config.hpp"
#include "redis_lite/logger.hpp"
#include <cstdlib>
#include <getopt.h>

void printUsage(const char *prog)
{
    std::cout << "Usage: " << prog << " [-p port] [-w wal_file] [-t threads]\n";
}

int main(int argc, char *argv[])
{
    Config config;
    int opt;
    while ((opt = getopt(argc, argv, "p:w:t:h")) != -1)
    {
        switch (opt)
        {
        case 'p':
            config.port = std::stoi(optarg);
            break;
        case 'w':
            config.wal_file = optarg;
            break;
        case 't':
            config.thread_pool_size = std::stoul(optarg);
            break;
        case 'h':
        default:
            printUsage(argv[0]);
            return 0;
        }
    }

    Server server(config);
    server.run();
    return 0;
}