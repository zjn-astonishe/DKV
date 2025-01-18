#include "client.h"
#include <iostream>

namespace kvstore
{

    KVClient::KVClient(std::shared_ptr<grpc::Channel> channel, size_t cache_capacity)
        : stub_(kvstore::KVStoreRPC::NewStub(channel)), cache_(cache_capacity) {}

    int64_t KVClient::getVersion()
    {
        return current_version;
    }

    grpc::Status KVClient::put(const std::string &key, const std::string &value)
    {
        kvstore::PutRequest request;
        kvstore::PutResponse response;
        grpc::ClientContext context;

        request.set_key(key);
        request.set_value(value);
        {
            std::lock_guard<std::mutex> lock(version_mutex);
            // std::cout << current_version << std::endl;
            request.set_version(current_version++);
        }
        grpc::Status status = stub_->Put(&context, request, &response);
        if (status.ok())
        {
            if (response.success())
            {
                std::cout << "Put operation successful." << std::endl;
                cache_.set(key, value, response.version());
            }
            else
            {
                std::cerr << "Put failed: " << status.error_message() << std::endl;
                std::lock_guard<std::mutex> lock(version_mutex);
                current_version = response.version();
                std::cout << "Version conflict, please retry with new version: " << current_version << std::endl;
            }
        }
        return status;
    }

    grpc::Status KVClient::get(const std::string &key, std::string &value, int64_t &version)
    {
        if (cache_.get(key, value, version) && current_version - version < 2)
        {
            return grpc::Status::OK; // 如果缓存有数据直接返回
        }

        kvstore::GetRequest request;
        kvstore::GetResponse response;
        grpc::ClientContext context;

        request.set_key(key);

        grpc::Status status = stub_->Get(&context, request, &response);
        if (status.ok() && response.found())
        {
            value = response.value();
            version = response.version();
            // std::cout << "v" << response.version() << std::endl;
            current_version = version + 1;
            return grpc::Status::OK;
        }
        else
        {
            std::cerr << "Get failed: " << status.error_message() << std::endl;
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Key not found");
        }
    }

    grpc::Status KVClient::del(const std::string &key)
    {
        kvstore::DeleteRequest request;
        kvstore::DeleteResponse response;
        grpc::ClientContext context;

        request.set_key(key);

        grpc::Status status = stub_->Del(&context, request, &response);
        if (status.ok() && response.success())
        {
            std::cout << "Del operation successful." << std::endl;
            cache_.clear(key);
            return grpc::Status::OK;
        }
        else
        {
            std::cerr << "Del failed: " << status.error_message() << std::endl;
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Key not found");
        }
    }

} // namespace kvstore
