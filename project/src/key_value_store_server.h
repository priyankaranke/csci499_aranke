#include <glog/logging.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "key_value_store.grpc.pb.h"
#include "key_value_store.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using kvstore::GetReply;
using kvstore::GetRequest;
using kvstore::KeyValueStore;
using kvstore::PutReply;
using kvstore::PutRequest;
using kvstore::RemoveReply;
using kvstore::RemoveRequest;

class KeyValueStoreServer final : public KeyValueStore::Service {
 public:
  // prevent default constructor
  KeyValueStoreServer() = delete;

  // creates a kv_store_ backed by filename
  KeyValueStoreServer(const std::string& filename);

  Status put(ServerContext* context, const PutRequest* request,
             PutReply* response) override;
  Status get(ServerContext* context,
             ServerReaderWriter<GetReply, GetRequest>* stream) override;
  Status remove(ServerContext* context, const RemoveRequest* request,
                RemoveReply* response) override;
  // method that initializes the latest warble as 0
  Status setup(); 
  // writes contents of map in kv_store_ to file. Called on SIGTERM
  void writeToFile();
 private:
  KvStore kv_store_;
};
