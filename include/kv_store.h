#ifndef KV_STORE_H
#define KV_STORE_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <utility> // for std::pair
#include <stdexcept>

namespace kvstore
{
    class NodeInfo
    {
    public:
        NodeInfo(const std::string& node_name, const std::string& node_address);
        ~NodeInfo();
        std::string get_name();
        std::string get_address();

    private:
        std::string node_name_;
        std::string node_address_;
    };

    class KVStore
    {
    public:
        KVStore(const NodeInfo &node_info);
        ~KVStore();
        bool put(const std::string &key, const std::string &value);
        bool get(const std::string &key, std::string &value);
        bool del(const std::string &key);

        NodeInfo get_nodeinfo();

    private:
        NodeInfo node_info_;
        std::unordered_map<std::string, std::string> store;
        std::mutex store_mutex;
    };

    // Function to parse host and port from a string in "host:port" format
    std::pair<std::string, int> parse_host_port(const std::string &input);
}

#endif
