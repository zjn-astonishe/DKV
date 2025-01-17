#include "server.h"
#include <spdlog/spdlog.h>

namespace kvstore
{

    KVStoreServiceImpl::KVStoreServiceImpl(const NodeInfo &node_info, const std::vector<NodeInfo> &nodes_map) : store_(node_info), nodes_map_(nodes_map)
    {
        for (auto i = nodes_map_.begin(); i != nodes_map_.end(); i++)
        {
            hash_ring_.addNode(i->get_name());
        }
    }

    KVStoreServiceImpl::~KVStoreServiceImpl()
    {
    }

    grpc::Status KVStoreServiceImpl::Put(grpc::ServerContext *context, const kvstore::PutRequest *request, kvstore::PutResponse *response)
    {
        std::string node = hash_ring_.getNode(request->key());
        // 如果当前节点负责存储
        if (node == store_.get_nodeinfo().get_name())
        {
            store_.put(request->key(), request->value());
            response->set_success(true);
            return grpc::Status::OK;
        }
        // 如果当前节点不负责存储，则转发请求给其他节点
        std::string target_address; // 使用 node_map 查询目标节点的地址
        for (auto i = nodes_map_.begin(); i != nodes_map_.end(); i++)
        {
            if (i->get_name() == node)
            {
                target_address = i->get_address();
            }
        }
        if (target_address.empty())
        {
            // 如果没有找到目标节点的地址，返回错误
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Target node not found");
        }

        // 创建 gRPC 客户端来转发请求
        auto channel = grpc::CreateChannel(target_address, grpc::InsecureChannelCredentials());
        kvstore::KVStoreRPC::Stub stub(channel);

        // 构建转发请求
        kvstore::PutRequest forward_request;
        forward_request.set_key(request->key());
        forward_request.set_value(request->value());

        kvstore::PutResponse forward_response;
        grpc::ClientContext client_context;

        // 转发请求给目标节点
        grpc::Status status = stub.Put(&client_context, forward_request, &forward_response);

        if (status.ok() && forward_response.success())
        {
            response->set_success(true);
            return grpc::Status::OK;
        }
        else
        {
            // 转发失败，返回错误
            response->set_success(false);
            return grpc::Status(grpc::StatusCode::INTERNAL, "Forwarding request failed");
        }
    }

    grpc::Status KVStoreServiceImpl::Get(grpc::ServerContext *context, const kvstore::GetRequest *request, kvstore::GetResponse *response)
    {
        std::string node = hash_ring_.getNode(request->key());
        if (node == store_.get_nodeinfo().get_name())
        {
            std::string value;
            if (store_.get(request->key(), value))
            {
                response->set_value(value);
                response->set_found(true);
            }
            else
            {
                response->set_found(false);
            }
            return grpc::Status::OK;
        }
        // 如果当前节点不负责存储，则转发请求给其他节点
        std::string target_address; // 使用 node_map 查询目标节点的地址
        for (auto i = nodes_map_.begin(); i != nodes_map_.end(); i++)
        {
            if (i->get_name() == node)
            {
                target_address = i->get_address();
            }
        }
        if (target_address.empty())
        {
            // 如果没有找到目标节点的地址，返回错误
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Target node not found");
        }

        // 创建 gRPC 客户端来转发请求
        auto channel = grpc::CreateChannel(target_address, grpc::InsecureChannelCredentials());
        kvstore::KVStoreRPC::Stub stub(channel);

        // 构建转发请求
        kvstore::GetRequest forward_request;
        forward_request.set_key(request->key());

        kvstore::GetResponse forward_response;
        grpc::ClientContext client_context;

        // 转发请求给目标节点
        grpc::Status status = stub.Get(&client_context, forward_request, &forward_response);

        if (status.ok() && forward_response.found())
        {
            response->set_value(forward_response.value());
            response->set_found(true);
            return grpc::Status::OK;
        }
        else
        {
            // 转发失败，返回错误
            response->set_found(false);
            return grpc::Status(grpc::StatusCode::INTERNAL, "Forwarding request failed");
        }
    }

    grpc::Status KVStoreServiceImpl::Del(grpc::ServerContext *context, const kvstore::DeleteRequest *request, kvstore::DeleteResponse *response)
    {
        std::string node = hash_ring_.getNode(request->key());
        if (node == store_.get_nodeinfo().get_name())
        {
            if (store_.del(request->key()))
            {
                response->set_success(true);
            }
            else
            {
                response->set_success(false);
            }
            return grpc::Status::OK;
        }
        // 如果当前节点不负责存储，则转发请求给其他节点
        std::string target_address; // 使用 node_map 查询目标节点的地址
        for (auto i = nodes_map_.begin(); i != nodes_map_.end(); i++)
        {
            if (i->get_name() == node)
            {
                target_address = i->get_address();
            }
        }
        if (target_address.empty())
        {
            // 如果没有找到目标节点的地址，返回错误
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Target node not found");
        }

        // 创建 gRPC 客户端来转发请求
        auto channel = grpc::CreateChannel(target_address, grpc::InsecureChannelCredentials());
        kvstore::KVStoreRPC::Stub stub(channel);

        // 构建转发请求
        kvstore::DeleteRequest forward_request;
        forward_request.set_key(request->key());

        kvstore::DeleteResponse forward_response;
        grpc::ClientContext client_context;

        // 转发请求给目标节点
        grpc::Status status = stub.Del(&client_context, forward_request, &forward_response);

        if (status.ok() && forward_response.success())
        {
            response->set_success(true);
            return grpc::Status::OK;
        }
        else
        {
            // 转发失败，返回错误
            response->set_success(false);
            return grpc::Status(grpc::StatusCode::INTERNAL, "Forwarding request failed");
        }
    }
}
