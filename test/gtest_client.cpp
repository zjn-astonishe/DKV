#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include "kvstore.grpc.pb.h"
#include "client.h"
#include <thread>

// 模拟 PUT 请求，支持多个键值对
TEST(KVStoreTest, TestPut)
{
    std::string server_address("localhost:50051");
    kvstore::KVClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::vector<std::pair<std::string, std::string>> key_value_pairs = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };

    // 执行多个 PUT 操作
    for (const auto& pair : key_value_pairs) {
        grpc::Status status = client.put(pair.first, pair.second);
        ASSERT_TRUE(status.ok()) << "PUT failed for key: " << pair.first;
    }
}

// 模拟 GET 请求，验证多个键值对
TEST(KVStoreTest, TestGet)
{
    std::string server_address("localhost:50051");
    kvstore::KVClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::vector<std::pair<std::string, std::string>> key_value_pairs = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };

    // 执行多个 GET 操作
    for (const auto& pair : key_value_pairs) {
        std::string value;
        int64_t version;
        grpc::Status status = client.get(pair.first, value, version);
        ASSERT_TRUE(status.ok()) << "GET failed for key: " << pair.first;
        ASSERT_EQ(value, pair.second) << "Expected value: " << pair.second << " but got: " << value;
    }
}

// 模拟 DEL 请求，删除多个键
TEST(KVStoreTest, TestDel)
{
    std::string server_address("localhost:50051");
    kvstore::KVClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::vector<std::string> keys_to_delete = {"key1", "key2", "key3"};

    // 执行多个 DEL 操作
    for (const auto& key : keys_to_delete) {
        grpc::Status status = client.del(key);
        ASSERT_TRUE(status.ok()) << "DEL failed for key: " << key;
    }
}

// 测试并发 PUT 请求(版本冲突后重试)
TEST(KVStoreTest, TestConcurrentPut)
{
    std::string server_address("localhost:50051");
    kvstore::KVClient client1(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);
    kvstore::KVClient client2(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::string key = "key1";
    std::string value1 = "value1";
    std::string value2 = "value2";

    // 使用两个线程模拟并发的 PUT 请求
    std::thread t1([&]() {
        grpc::Status status = client1.put(key, value1);
        if (!status.ok()) {
            std::cerr << "PUT failed for client1: " << key << std::endl;
        }
    });

    std::thread t2([&]() {
        grpc::Status status = client2.put(key, value2);
        if (!status.ok()) {
            std::cerr << "PUT failed for client1: " << key << std::endl;
        }
    });

    std::thread t3([&]() {
        grpc::Status status = client2.put(key, value2);
        if (!status.ok()) {
            std::cerr << "PUT failed for client1: " << key << std::endl;
        }
    });

    // 等待两个线程完成
    t1.join();
    t2.join();
    t3.join();
}

// 测试并发 GET 请求
TEST(KVStoreTest, TestConcurrentGet)
{
    std::string server_address("localhost:50051");
    kvstore::KVClient client1(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);
    kvstore::KVClient client2(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::string key = "key1";
    std::string expected_value = "value2";

    // 使用两个线程模拟并发的 GET 请求
    std::thread t1([&]() {
        std::string value;
        int64_t version;
        grpc::Status status = client1.get(key, value, version);
        ASSERT_TRUE(status.ok()) << "GET failed for key: " << key;
        ASSERT_EQ(value, expected_value) << "Expected value: " << expected_value << " but got: " << value;
    });

    std::thread t2([&]() {
        std::string value;
        int64_t version;
        grpc::Status status = client2.get(key, value, version);
        ASSERT_TRUE(status.ok()) << "GET failed for key: " << key;
        ASSERT_EQ(value, expected_value) << "Expected value: " << expected_value << " but got: " << value;
    });

    // 等待两个线程完成
    t1.join();
    t2.join();
}

// 测试并发 DEL 请求
TEST(KVStoreTest, TestConcurrentDel)
{
    std::string server_address("localhost:50051");
    kvstore::KVClient client1(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);
    kvstore::KVClient client2(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::string key = "key1";
    std::string value1 = "value1";
    std::string value2 = "value2";

    // 使用两个线程模拟并发的 PUT 请求
    std::thread t1([&]() {
        grpc::Status status = client1.del(key);
        if (!status.ok()) {
            std::cerr << "DEL failed for client2: " << key << std::endl;
        }
    });

    std::thread t2([&]() {
        grpc::Status status = client2.del(key);
        if (!status.ok()) {
            std::cerr << "DEL failed for client2: " << key << std::endl;
        }
    });

    // 等待两个线程完成
    t1.join();
    t2.join();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
