// #include <glog/logging.h>
#include <grpcpp/grpcpp.h>
#include <iostream>

#include "key_value_store.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using kvstore::GetReply;
using kvstore::GetRequest;
using kvstore::KeyValueStore;
using kvstore::PutReply;
using kvstore::PutRequest;
using kvstore::RemoveReply;
using kvstore::RemoveRequest;

class Database {
 public:
  virtual void put(const std::string& key, const std::string& value) = 0;
  virtual void remove(const std::string& key) = 0;
  virtual std::vector<GetReply> get(const std::string& key) = 0;
};

class KeyValueStoreClient : protected Database {
 public:
  KeyValueStoreClient() = delete;
  KeyValueStoreClient(std::shared_ptr<Channel> channel);

  void put(const std::string& key, const std::string& value);
  void remove(const std::string& key);
  std::vector<GetReply> get(const std::string& key);

 private:
  std::unique_ptr<KeyValueStore::Stub> stub_;
};