#include "redis_lite/store.hpp"
#include <mutex>

void Store::set(const std::string &key, const std::string &value)
{
    std::unique_lock lock(mutex_);
    data_[key] = value;
}
std::optional<std::string> Store::get(const std::string &key) const
{
    std::shared_lock lock(mutex_);
    auto it = data_.find(key);
    if (it != data_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

bool Store::del(const std::string &key)
{
    std::unique_lock lock(mutex_);
    return data_.erase(key) > 0;
}