#include "kv_store.h"

namespace kvstore
{
    NodeInfo::NodeInfo(const std::string& node_name, const std::string& node_address) : node_name_(node_name), node_address_(node_address)
    {
    }
    NodeInfo::~NodeInfo()
    {
    }

    std::string NodeInfo::get_name()
    {
        return node_name_;
    }

    std::string NodeInfo::get_address()
    {
        return node_address_;
    }

    NodeInfo KVStore::get_nodeinfo()
    {
        return node_info_;
    }

    KVStore::KVStore(const NodeInfo &node_info) : node_info_(node_info)
    {
    }

    KVStore::~KVStore()
    {
    }

    bool KVStore::put(const std::string &key, const std::string &value)
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        store[key] = value;
        return true;
    }

    bool KVStore::get(const std::string &key, std::string &value)
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        auto it = store.find(key);
        if (it != store.end())
        {
            value = it->second;
            return true;
        }
        return false;
    }

    bool KVStore::del(const std::string &key)
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        auto it = store.find(key);
        if (it != store.end())
        {
            store.erase(it);
            return true;
        }
        return false;
    }

    // Function to parse host and port from a string in "host:port" format
    std::pair<std::string, int> parse_host_port(const std::string &input)
    {
        if (input.empty())
        {
            throw std::invalid_argument("Input string is empty");
        }

        size_t colon_pos = input.find(':');
        if (colon_pos == std::string::npos || colon_pos == 0 || colon_pos == input.size() - 1)
        {
            throw std::invalid_argument("Invalid host:port format");
        }

        std::string host = input.substr(0, colon_pos);
        try
        {
            int port = std::stoi(input.substr(colon_pos + 1));
            return std::make_pair(host, port);
        }
        catch (const std::exception &)
        {
            throw std::invalid_argument("Invalid port number");
        }
    }

} // namespace kvstore
