#include <grpcpp/grpcpp.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include "server.h"

// 帮助信息
void PrintUsage()
{
    std::cout << "Usage: ./server --node_count <node_count>" << std::endl;
}

void StartServer(const std::string &node_name, const std::string &address, std::vector<kvstore::NodeInfo> other_nodes)
{
    kvstore::NodeInfo node(node_name, address);
    kvstore::KVStoreServiceImpl service(node, other_nodes);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server " << node_name << " listening on " << address << std::endl;

    server->Wait();
}

int main(int argc, char **argv)
{
    if (argc != 3)
    { // 检查参数数量
        PrintUsage();
        return -1;
    }

    int node_count = 0;
    std::string host;
    int port = 0;

    // 解析命令行参数
    for (int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == "--node_count" && i + 1 < argc)
        {
            node_count = std::stoi(argv[i + 1]);
            i++;
        }
        else
        {
            PrintUsage();
            return -1;
        }
    }

    if (node_count <= 0)
    {
        PrintUsage();
        return -1;
    }

    // 创建节点名称的vector
    std::vector<kvstore::NodeInfo> nodes;
    for (int i = 1; i <= node_count; ++i)
    {
        std::string node_name = "node" + std::to_string(i);
        std::string node_host = "0.0.0.0";
        int node_port = 50050 + i;
        kvstore::NodeInfo node(node_name, node_host + ":" + std::to_string(node_port));
        nodes.push_back(node); // 生成节点名称
    }

    // 启动多个节点
    std::vector<std::thread> threads;
    for (int i = 0; i < node_count; ++i)
    {
        int node_port = port + i; // 为每个节点分配不同的端口
        threads.push_back(std::thread(StartServer, nodes[i].get_name(), nodes[i].get_address(), nodes));
    }

    // 等待所有线程完成
    for (auto &t : threads)
    {
        t.join();
    }

    return 0;
}
