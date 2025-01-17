#include "client.h"
#include <iostream>

namespace kvstore
{

    KVClient::KVClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(kvstore::KVStoreRPC::NewStub(channel)) {}

    bool KVClient::put(const std::string &key, const std::string &value)
    {
        kvstore::PutRequest request;
        kvstore::PutResponse response;
        grpc::ClientContext context;

        request.set_key(key);
        request.set_value(value);

        grpc::Status status = stub_->Put(&context, request, &response);
        if (status.ok())
        {
            return response.success();
        }
        else
        {
            std::cerr << "Put failed: " << status.error_message() << std::endl;
            return false;
        }
    }

    bool KVClient::get(const std::string &key, std::string &value)
    {
        kvstore::GetRequest request;
        kvstore::GetResponse response;
        grpc::ClientContext context;

        request.set_key(key);

        grpc::Status status = stub_->Get(&context, request, &response);
        if (status.ok() && response.found())
        {
            value = response.value();
            return true;
        }
        else
        {
            std::cerr << "Get failed: " << status.error_message() << std::endl;
            return false;
        }
    }

    bool KVClient::del(const std::string &key)
    {
        kvstore::DeleteRequest request;
        kvstore::DeleteResponse response;
        grpc::ClientContext context;

        request.set_key(key);

        grpc::Status status = stub_->Del(&context, request, &response);
        if (status.ok())
        {
            return response.success();
        }
        else
        {
            std::cerr << "Del failed: " << status.error_message() << std::endl;
            return false;
        }
    }

} // namespace kvstore
