// #include <glog/logging.h>
#include <grpcpp/grpcpp.h>

#include "database.h"

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

class KeyValueStoreClient : public Database {
 public:
  KeyValueStoreClient() = delete;
  KeyValueStoreClient(std::shared_ptr<Channel> channel);

  void put(const std::string& key, const std::string& value) override;
  void remove(const std::string& key) override;
  std::vector<GetReply> get(const std::string& key) override;

 private:
  std::unique_ptr<KeyValueStore::Stub> stub_;
};