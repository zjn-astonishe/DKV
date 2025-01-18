#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <vector>
#include "kvstore.grpc.pb.h"
#include "client.h"

std::atomic<int> success_count(0);
std::atomic<int> fail_count(0);

// 执行 PUT 操作的线程
void perform_put(kvstore::KVClient &client, const std::string &key, const std::string &value) {
    grpc::Status status = client.put(key, value);
    if (status.ok()) {
        success_count++;
    } else {
        fail_count++;
    }
}

// 执行 GET 操作的线程
void perform_get(kvstore::KVClient &client, const std::string &key) {
    std::string value;
    int64_t version;
    grpc::Status status = client.get(key, value, version);
    if (status.ok()) {
        success_count++;
    } else {
        fail_count++;
    }
}

void perform_del(kvstore::KVClient &client, const std::string &key) {
    std::string value;
    int64_t version;
    grpc::Status status = client.del(key);
    if (status.ok()) {
        success_count++;
    } else {
        fail_count++;
    }
}

// 测试 PUT 操作的并发压力
TEST(KVStoreTest, TestPutPressure) {
    std::string server_address("localhost:50051");
    kvstore::KVClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::vector<std::thread> threads;

    // 模拟 1000 个并发 PUT 操作
    for (int i = 0; i < 100; ++i) {
        threads.push_back(std::thread(perform_put, std::ref(client), "key" + std::to_string(i), "value" + std::to_string(i)));
    }

    // 等待所有线程完成
    for (auto &t : threads) {
        t.join();
    }

    // 输出测试结果
    std::cout << "PUT requests: " << success_count.load() << " succeeded, "
              << fail_count.load() << " failed." << std::endl;
}

// 测试 GET 操作的并发压力
TEST(KVStoreTest, TestGetPressure) {
    std::string server_address("localhost:50051");
    kvstore::KVClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::vector<std::thread> threads;

    // 模拟 1000 个并发 GET 操作
    for (int i = 0; i < 100; ++i) {
        threads.push_back(std::thread(perform_get, std::ref(client), "key" + std::to_string(i)));
    }

    // 等待所有线程完成
    for (auto &t : threads) {
        t.join();
    }

    // 输出测试结果
    std::cout << "GET requests: " << success_count.load() << " succeeded, "
              << fail_count.load() << " failed." << std::endl;
}

// 测试 DEL 操作的并发压力
TEST(KVStoreTest, TestDelPressure) {
    std::string server_address("localhost:50051");
    kvstore::KVClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::vector<std::thread> threads;

    // 模拟 1000 个并发 GET 操作
    for (int i = 0; i < 100; ++i) {
        threads.push_back(std::thread(perform_del, std::ref(client), "key" + std::to_string(i)));
    }

    // 等待所有线程完成
    for (auto &t : threads) {
        t.join();
    }

    // 输出测试结果
    std::cout << "DEL requests: " << success_count.load() << " succeeded, "
              << fail_count.load() << " failed." << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
