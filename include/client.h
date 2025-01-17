#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <grpcpp/grpcpp.h>
#include "kvstore.grpc.pb.h"

namespace kvstore
{
    class KVClient
    {
    public:
        KVClient(std::shared_ptr<grpc::Channel> channel);
        bool put(const std::string &key, const std::string &value);
        bool get(const std::string &key, std::string &value);
        bool del(const std::string &key);

    private:
        std::unique_ptr<kvstore::KVStoreRPC::Stub> stub_;
    };

} // namespace kvstore

#endif
