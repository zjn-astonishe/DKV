#ifndef SERVER_H
#define SERVER_H

#include <grpcpp/grpcpp.h>
#include "kvstore.grpc.pb.h"
#include "kv_store.h"
#include "consistency_hash.h"
#include <vector>

namespace kvstore
{
    class KVStoreServiceImpl final : public KVStoreRPC::Service
    {
    public:
        KVStoreServiceImpl(const NodeInfo& node_info, const std::vector<NodeInfo>& nodes_map = {});
        ~KVStoreServiceImpl();
        grpc::Status Put(grpc::ServerContext *context, const PutRequest *request, PutResponse *response) override;
        grpc::Status Get(grpc::ServerContext *context, const GetRequest *request, GetResponse *response) override;
        grpc::Status Del(grpc::ServerContext *context, const DeleteRequest *request, DeleteResponse *response) override;

    private:
        KVStore store_;
        std::vector<NodeInfo> nodes_map_;
        ConsistencyHash hash_ring_;
    };

}

#endif
