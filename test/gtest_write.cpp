#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include "kvstore.grpc.pb.h"
#include "client.h"

std::atomic<int> success_count(0); // 用于记录成功的请求次数
std::atomic<int> fail_count(0);    // 用于记录失败的请求次数

// Client1 执行 PUT 操作，更新 key1 的值
void perform_put(kvstore::KVClient &client, const std::string &key, const std::string &value)
{
    grpc::Status status = client.put(key, value);
    // if (status.ok()) {
    //     success_count++;
    // } else {
    //     fail_count++;
    // }
}

// Client2 和 Client3 执行 GET 操作，确保 key1 的值是单调递增的
void perform_get(kvstore::KVClient &client, const std::string &key, std::string &last_value, int64_t last_version)
{
    std::string value;
    int64_t version;
    grpc::Status status = client.get(key, value, version);

    if (status.ok())
    {
        if (version > last_version)
        {
            success_count++;    // 表示值正确更新
            last_value = value; // 更新为最新的值
            std::cout << "value: " << value << " version: " << version << std::endl;
        }
        else
        {
            fail_count++; // 如果值不单调递增，标记为失败
        }
    }
}

// 测试单调写的功能
TEST(KVStoreTest, TestMonotonicWrites)
{
    std::string server_address("localhost:50051");
    kvstore::KVClient client1(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);
    kvstore::KVClient client2(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);
    kvstore::KVClient client3(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 10);

    std::string key = "key1";

    // Client2 和 Client3 用于读取 key1
    std::string last_value = "0"; // 初始化值，保证第一个 GET 是有效的
    int64_t last_version = -1;

    std::vector<std::thread> threads;

    // 启动 Client1 执行 10 次 PUT 操作
    for (int i = 0; i < 10; ++i)
    {
        threads.push_back(std::thread(perform_put, std::ref(client1), key, "value" + std::to_string(i)));
        // std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 模拟 3 毫秒的延时
        threads.push_back(std::thread(perform_get, std::ref(client2), key, std::ref(last_value), last_version));
        threads.push_back(std::thread(perform_get, std::ref(client3), key, std::ref(last_value), last_version));
        // std::this_thread::sleep_for(std::chrono::milliseconds(2)); // 模拟 3 毫秒的延时
    }
    // 等待所有线程完成
    for (auto &t : threads)
    {
        t.join();
    }

    // 输出测试结果
    std::cout << "Total requests: " << success_count.load() << " succeeded, "
              << fail_count.load() << " failed." << std::endl;

    ASSERT_GT(success_count.load(), 0); // 确保有成功的操作
    ASSERT_EQ(fail_count.load(), 0);    // 确保没有失败的操作

    // 删除 key1
    grpc::Status status = client1.get(key, last_value, last_version);
    // status = client2.put(key, "value1");
    status = client1.del(key);
    ASSERT_TRUE(status.ok()) << "DEL failed for key: " << key;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
