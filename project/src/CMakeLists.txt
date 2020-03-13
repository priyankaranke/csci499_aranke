cmake_minimum_required(VERSION 3.10)

project(Warble)

########## HANDLE GRPC/PROTOCOL BUFFER STUFF ##########

# GRPC and Protocol Buffers libraries location
list(APPEND CMAKE_PREFIX_PATH "/opt/grpc" "/opt/protobuf")

# Cmake find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

set(PROTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/../protos/key_value_store.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/../protos/func.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/../protos/warble.proto
)

set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/protos)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
include_directories(${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})

########## HANDLE GFLAGS ##########
find_package (gflags REQUIRED)
include_directories (${gflags_INCLUDE_DIR})

########## DEFINITIONS FOR THIS CMAKELIST FILE ##########

# set key_value_store sources
set(kv_SRCS
    key_value_store.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

# set key_value_store_client sources
set(kv_client_SRCS
    key_value_store_client.cpp
    database.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

# set func sources
set(func_SRCS
    func.cpp
    key_value_store_client.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

set(warble_functionality_SRCS
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

set(func_client_SRCS
    func_client.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

########## EXPORT key_value_store AS LIBRARY ##########

add_library(key_value_store STATIC 
    ${kv_SRCS}
)

target_link_libraries(key_value_store 
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

target_include_directories(key_value_store PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########## ADD key_value_store_main AS EXECUTABLE ##########
find_package(glog 0.3.5 REQUIRED)

# add the executable
add_executable(key_value_store_main
    key_value_store_server.cpp
    key_value_store_main.cpp
)

target_link_libraries(key_value_store_main
    key_value_store
    glog::glog
)

########## EXPORT key_value_store_client AS LIBRARY ##########

add_library(key_value_store_client STATIC 
    ${kv_client_SRCS}
)   

target_link_libraries(key_value_store_client 
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    glog::glog
)

########## EXPORT func AS LIBRARY ##########

add_library(func STATIC 
    ${func_SRCS}
)

target_link_libraries(func 
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

target_include_directories(func PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########## EXPORT warble_functionality AS LIBRARY ##########
add_library(warble_functionality STATIC 
    ${warble_functionality_SRCS}
)

target_link_libraries(warble_functionality 
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

target_include_directories(warble_functionality PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########## ADD func AS EXECUTABLE ##########
# add the executable
add_executable(func_main
    func_server.cpp
    func_main.cpp
)

target_link_libraries(func_main
    func
    warble_functionality
    glog::glog
)

########## EXPORT func_client AS LIBRARY ##########

add_library(func_client STATIC 
    ${func_client_SRCS}
)   

target_link_libraries(func_client 
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    glog::glog
)

########## ADD warble_main AS EXECUTABLE ##########
# add the executable
add_executable(warble_main
    warble_main.cpp
)

target_link_libraries(warble_main
    func_client
    gflags
)
