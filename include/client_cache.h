#ifndef KVCACHELRU_H
#define KVCACHELRU_H

#include <unordered_map>
#include <list>
#include <string>
#include <mutex>

namespace kvstore
{
    class KVCacheLRU
    {
    public:
        // 构造函数，接受缓存的最大容量
        KVCacheLRU(size_t capacity);

        // 获取缓存中的数据，返回值为 true 表示找到缓存并赋值
        bool get(const std::string &key, std::string &value, int64_t &version);

        // 设置缓存项
        void set(const std::string &key, const std::string &value, int64_t version);

        // 清除指定键的缓存
        void clear(const std::string &key);

    private:
        size_t capacity_;                                                                                                         // 缓存容量
        std::list<std::string> cache_list_;                                                                                       // 用于按访问顺序排列的键
        std::unordered_map<std::string, std::pair<std::pair<std::string, int64_t>, std::list<std::string>::iterator>> cache_map_; // 键与缓存条目的映射
        std::mutex mutex_;                                                                                                        // 保护缓存的线程安全
    };
}

#endif // KVCACHELRU_H
