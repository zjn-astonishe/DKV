#include "client_cache.h"

namespace kvstore
{

    KVCacheLRU::KVCacheLRU(size_t capacity) : capacity_(capacity) {}

    bool KVCacheLRU::get(const std::string &key, std::string &value, int64_t &version)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_map_.find(key);
        if (it != cache_map_.end())
        {
            // 移动到前面，表示最近访问
            cache_list_.splice(cache_list_.begin(), cache_list_, it->second.second);
            value = it->second.first.first;    // 提取缓存的值
            version = it->second.first.second; // 提取版本号
            return true;
        }
        return false;
    }

    void KVCacheLRU::set(const std::string &key, const std::string &value, int64_t version)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_map_.find(key);
        if (it != cache_map_.end())
        {
            // 更新缓存条目并移动到前面
            it->second.first.first = value;
            it->second.first.second = version;
            cache_list_.splice(cache_list_.begin(), cache_list_, it->second.second);
        }
        else
        {
            // 如果缓存已满，移除最少使用的元素
            if (cache_map_.size() >= capacity_)
            {
                const auto &oldest = cache_list_.back();
                cache_map_.erase(oldest);
                cache_list_.pop_back();
            }

            // 添加新条目到前面
            cache_list_.emplace_front(key);
            cache_map_[key] = {{value, version}, cache_list_.begin()};
        }
    }

    void KVCacheLRU::clear(const std::string &key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_map_.find(key);
        if (it != cache_map_.end())
        {
            cache_list_.erase(it->second.second);
            cache_map_.erase(it);
        }
    }

} // namespace kvstore
