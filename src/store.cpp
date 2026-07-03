#include "store.h"
#include <iostream>
#include <sstream>

KVStore::KVStore(const std::string &wal_path) : wal_path_(wal_path)
{
    recover_from_wal();
    wal_file_.open(wal_path_, std::ios::app);
}

void KVStore::recover_from_wal()
{
    std::ifstream infile(wal_path_);
    std::string command, key, value;
    while (infile >> command >> key >> value)
    {
        if (command == "SET")
        {
            map_[key] = value;
        }
    }
    std::cout << "Recovered data from WAL." << std::endl;
}

std::string KVStore::get(const std::string &key)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = map_.find(key);
    if (it != map_.end())
    {
        return it->second;
    }
    return "";
}

void KVStore::set(const std::string &key, const std::string &value)
{
    {
        std::lock_guard<std::mutex> wal_lock(wal_mutex_);
        wal_file_ << "SET " << key << " " << value << "\n";
        wal_file_.flush();
    }

    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        map_[key] = value;
    }
}