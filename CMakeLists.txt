cmake_minimum_required(VERSION 3.10)
project(MyDSMProject VERSION 1.0)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1  # Optionally pin to a specific version/release
)
FetchContent_MakeAvailable(googletest)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加 CMake 模块路径
list(APPEND CMAKE_MODULE_PATH "/usr/lib/cmake/grpc" "/usr/lib/cmake/protobuf")
# 设置 RPATH 为只包含系统路径
set(CMAKE_INSTALL_RPATH "/usr/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)

# 查找 gRPC 和 Protobuf 包
# set(CMAKE_FIND_DEBUG_MODE TRUE)
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(fmt REQUIRED)



# 添加 proto 文件生成规则
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kvstore.pb.cc
         ${CMAKE_CURRENT_BINARY_DIR}/kvstore.pb.h
         ${CMAKE_CURRENT_BINARY_DIR}/kvstore.grpc.pb.cc
         ${CMAKE_CURRENT_BINARY_DIR}/kvstore.grpc.pb.h
  COMMAND protoc
    --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
    --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
    --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
    --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kvstore.proto
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kvstore.proto
  COMMENT "Generating gRPC code from proto files"
)

add_custom_target(
  GenerateProto ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kvstore.pb.cc
          ${CMAKE_CURRENT_BINARY_DIR}/kvstore.grpc.pb.cc
)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test")

# 添加可执行文件
add_executable(test_server
  ${TEST_DIR}/test_server.cpp
  ${SRC_DIR}/server.cpp
  ${SRC_DIR}/kv_store.cpp
  ${SRC_DIR}/consistency_hash.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.grpc.pb.cc
)

add_executable(test_client
  ${TEST_DIR}/test_client.cpp
  ${SRC_DIR}/client.cpp
  ${SRC_DIR}/client_cache.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.grpc.pb.cc
)

add_executable(gtest_client
  ${TEST_DIR}/gtest_client.cpp
  ${SRC_DIR}/client.cpp
  ${SRC_DIR}/client_cache.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.grpc.pb.cc
)

add_executable(gtest_cache
  ${TEST_DIR}/gtest_cache.cpp
  ${SRC_DIR}/client.cpp
  ${SRC_DIR}/client_cache.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.grpc.pb.cc
)

add_executable(gtest_stress
  ${TEST_DIR}/gtest_stress.cpp
  ${SRC_DIR}/client.cpp
  ${SRC_DIR}/client_cache.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.grpc.pb.cc
)

add_executable(gtest_write
  ${TEST_DIR}/gtest_write.cpp
  ${SRC_DIR}/client.cpp
  ${SRC_DIR}/client_cache.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/kvstore.grpc.pb.cc
)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# 设置头文件搜索路径
target_include_directories(test_server PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${INCLUDE_DIR})
target_include_directories(test_client PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${INCLUDE_DIR})
target_include_directories(gtest_client PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${INCLUDE_DIR})
target_include_directories(gtest_cache PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${INCLUDE_DIR})
target_include_directories(gtest_stress PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${INCLUDE_DIR})
target_include_directories(gtest_write PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${INCLUDE_DIR})


# 链接 gRPC 和 Protobuf 库
target_link_libraries(test_server gRPC::grpc++ protobuf::libprotobuf fmt::fmt)
target_link_libraries(test_client gRPC::grpc++ protobuf::libprotobuf fmt::fmt)
target_link_libraries(gtest_client gRPC::grpc++ protobuf::libprotobuf fmt::fmt gtest_main)
target_link_libraries(gtest_cache gRPC::grpc++ protobuf::libprotobuf fmt::fmt gtest_main)
target_link_libraries(gtest_stress gRPC::grpc++ protobuf::libprotobuf fmt::fmt gtest_main)
target_link_libraries(gtest_write gRPC::grpc++ protobuf::libprotobuf fmt::fmt gtest_main)

# 确保生成的 proto 文件先于可执行文件构建
add_dependencies(test_server GenerateProto)
add_dependencies(test_client GenerateProto)
add_dependencies(gtest_client GenerateProto)
add_dependencies(gtest_stress GenerateProto)
add_dependencies(gtest_cache GenerateProto)
add_dependencies(gtest_write GenerateProto)

enable_testing()
include(GoogleTest)
gtest_discover_tests(gtest_client)
gtest_discover_tests(gtest_cache)
gtest_discover_tests(gtest_stress)
gtest_discover_tests(gtest_write)