#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include "client.h"

// 模拟 PUT 请求
TEST(KVStoreTest, TestCacheAndPutLRU)
{
    std::string server_address("localhost:50051");
    kvstore::KVClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 2);

    std::string key = "key1";
    std::string value1 = "value1";

    // 执行 PUT 操作
    grpc::Status status = client.put(key, value1);
    ASSERT_TRUE(status.ok()) << "PUT failed for key: " << key;

    // 执行 GET 操作，应该直接从缓存获取
    std::string value;
    int64_t version;
    status = client.get(key, value, version);
    ASSERT_TRUE(status.ok()) << "GET failed for key: " << key;
    ASSERT_EQ(value, value1) << "Expected value: " << value1 << " but got: " << value;

    status = client.del(key);
    ASSERT_TRUE(status.ok()) << "DEL failed for key: " << key;

    status = client.get(key, value, version);
    ASSERT_FALSE(status.ok()) << "GET failed for key: " << key;
    ASSERT_EQ(value, value1) << "Expected value: " << value1 << " but got: " << value;
}

// 测试缓存失效后的 GET 操作
TEST(KVStoreTest, TestCacheInvalidationLRU)
{
    std::string server_address("localhost:50051");
    kvstore::KVClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), 1);

    std::string key1 = "key1";
    std::string value1 = "value1";
    std::string key2 = "key2";
    std::string value2 = "value2";

    // 执行 PUT 操作
    grpc::Status status = client.put(key1, value1);
    ASSERT_TRUE(status.ok()) << "PUT failed for key: " << key1;

    // 执行 GET 操作，应该缓存获取
    std::string value;
    int64_t version;
    status = client.get(key1, value, version);
    ASSERT_TRUE(status.ok()) << "GET failed for key: " << key1;
    ASSERT_EQ(value, value1) << "Expected value: " << value1 << " but got: " << value;

    // 执行 PUT 操作，修改缓存数据
    status = client.put(key2, value2);
    ASSERT_TRUE(status.ok()) << "PUT failed for key: " << key2;

    // 再次执行 GET 操作，应该从缓存中获取更新的值
    status = client.get(key1, value, version);
    ASSERT_TRUE(status.ok()) << "GET failed for key: " << key1;
    ASSERT_EQ(value, value1) << "Expected value: " << value1 << " but got: " << value;

    status = client.del(key1);
    ASSERT_TRUE(status.ok()) << "DEL failed for key: " << key1;
    status = client.del(key2);
    ASSERT_TRUE(status.ok()) << "DEL failed for key: " << key2;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
