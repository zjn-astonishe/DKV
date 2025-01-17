#ifndef CONSISTENCY_HASH_H
#define CONSISTENCY_HASH_H

#include <string>
#include <vector>
#include <map>

class ConsistencyHash {
public:
    ConsistencyHash(int num_replicas = 3);
    void addNode(const std::string& node);
    std::string getNode(const std::string& key);

private:
    std::vector<std::string> nodes;
    std::map<int, std::string> ring;
    int num_replicas;
    unsigned int hash(const std::string& key);
};

#endif
