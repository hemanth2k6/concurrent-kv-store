#include "store.h"
#include "server.h"

int main()
{
    KVStore store("kvstore.wal");
    Server server(6379, store);
    server.start();
    return 0;
}