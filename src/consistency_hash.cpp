#include "consistency_hash.h"
#include <iostream>
#include <functional>

ConsistencyHash::ConsistencyHash(int num_replicas) : num_replicas(num_replicas) {}

unsigned int ConsistencyHash::hash(const std::string &key)
{
    return std::hash<std::string>{}(key);
}

void ConsistencyHash::addNode(const std::string &node)
{
    int hash_val = hash(node);
    // std::cout << "hash_val: " << hash_val << std::endl;
    ring[hash_val] = node;
    nodes.push_back(node);
}

std::string ConsistencyHash::getNode(const std::string &key)
{
    if (ring.empty())
        return "";
    int hash_val = hash(key);
    // std::cout << "hash_val for key: " << hash_val << std::endl;
    auto it = ring.lower_bound(hash_val);
    if (it == ring.end())
    {
        it = ring.begin();
    }
    // std::cout << it->second << std::endl;
    return it->second;
}
