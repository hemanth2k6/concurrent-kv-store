#pragma once
#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <mutex>
#include <fstream>
class KVStore
{
public:
    KVStore(const std::string &wal_path);
    std::string get(const std::string &key);
    void set(const std::string &key, const std::string &value);

private:
    void recover_from_wal();
    std::unordered_map<std::string, std::string> map_;
    mutable std::shared_mutex mutex_;
    std::string wal_path_;
    std::ofstream wal_file_;
    std::mutex wal_mutex_;
};