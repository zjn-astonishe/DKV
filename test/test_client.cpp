#include <grpcpp/grpcpp.h>
#include "client.h"
#include "kvstore.grpc.pb.h"
#include <iostream>
#include <thread>
#include <mutex>

std::mutex print_mutex; // 用于同步输出，避免多个线程打印混乱

// 用于模拟客户端请求
void perform_put(kvstore::KVClient& client, const std::string& key, const std::string& value) {
    bool success = client.put(key, value);
    {
        std::lock_guard<std::mutex> lock(print_mutex); // 保证线程安全地输出
        if (success) {
            std::cout << "PUT successful for key: " << key << " with value: " << value << std::endl;
        } else {
            std::cout << "PUT failed for key: " << key << std::endl;
        }
    }
}

void perform_get(kvstore::KVClient& client, const std::string& key) {
    std::string value;
    bool success = client.get(key, value);
    {
        std::lock_guard<std::mutex> lock(print_mutex);
        if (success) {
            std::cout << "GET successful for key: " << key << ", value: " << value << std::endl;
        } else {
            std::cout << "GET failed for key: " << key << std::endl;
        }
    }
}

void perform_del(kvstore::KVClient& client, const std::string& key) {
    bool success = client.del(key);
    {
        std::lock_guard<std::mutex> lock(print_mutex);
        if (success) {
            std::cout << "DEL successful for key: " << key << std::endl;
        } else {
            std::cout << "DEL failed for key: " << key << std::endl;
        }
    }
}

int main() {
    std::string server_address("localhost:50051");
    kvstore::KVClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    // 启动多个线程，模拟两个客户端并发操作
    std::thread client1_put_thread(perform_put, std::ref(client), "key3", "value1");
    std::thread client2_put_thread(perform_put, std::ref(client), "key2", "value2");

    // 等待两个客户端线程完成 PUT 操作
    client1_put_thread.join();
    client2_put_thread.join();

    // 启动两个线程进行 GET 操作，模拟两个客户端并发读取
    std::thread client1_get_thread(perform_get, std::ref(client), "key3");
    std::thread client2_get_thread(perform_get, std::ref(client), "key2");

    // 等待两个客户端线程完成 GET 操作
    client1_get_thread.join();
    client2_get_thread.join();

    // 启动两个线程进行 DEL 操作，模拟两个客户端并发删除
    std::thread client1_del_thread(perform_del, std::ref(client), "key3");
    std::thread client2_del_thread(perform_del, std::ref(client), "key2");

    // 等待两个客户端线程完成 DEL 操作
    client1_del_thread.join();
    client2_del_thread.join();

    return 0;
}
