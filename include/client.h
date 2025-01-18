#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <grpcpp/grpcpp.h>
#include "kvstore.grpc.pb.h"
#include "client_cache.h"

namespace kvstore
{
    class KVClient
    {
    public:
        KVClient(std::shared_ptr<grpc::Channel> channel, size_t cache_capacity);
        grpc::Status put(const std::string &key, const std::string &value);
        grpc::Status get(const std::string &key, std::string &value, int64_t &version);
        grpc::Status del(const std::string &key);
        int64_t getVersion();

    private:
        std::unique_ptr<kvstore::KVStoreRPC::Stub> stub_;
        int64_t current_version = 0;
        std::mutex version_mutex;
        KVCacheLRU cache_; // LRU 缓存实例
    };

} // namespace kvstore

#endif
