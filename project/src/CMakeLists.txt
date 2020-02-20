cmake_minimum_required(VERSION 3.10)

# set the project name
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
)

set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/protos)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
include_directories(${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})

########## DEFINITIONS FOR THIS CMAKELIST FILE ##########

# set Warble sources
set(Warble_SRCS
    key_value_store.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

########## EXPORT key_value_store AS LIBRARY ##########

add_library(Warble STATIC 
    ${Warble_SRCS}
)

target_link_libraries(Warble 
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

target_include_directories(Warble PUBLIC
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
    Warble
    glog::glog
)