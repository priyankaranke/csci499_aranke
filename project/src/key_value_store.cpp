#include <iostream>

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "../protos/key_value_store.grpc.pb.h"
#include "key_value_store_server.h"

#include <glog/logging.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::ServerContext;
using grpc::Status;

using kvstore::KeyValueStore;
using kvstore::PutRequest;
using kvstore::PutReply;
using kvstore::GetRequest;
using kvstore::GetReply;
using kvstore::RemoveRequest;
using kvstore::RemoveReply;

class KeyValueStoreImpl final : public KeyValueStore::Service {
 public:
   KeyValueStoreImpl() {
     std::unique_ptr<KeyValueStoreBackend> kv_store_backend_ { new KeyValueStoreBackend() };
   };

 private:
   Status put(ServerContext* context, const PutRequest* request, PutReply* response) override {
     return Status::OK;
   }

   Status get(ServerContext* context, ServerReaderWriter<GetReply, GetRequest>* stream) override {
     return Status::OK;
   }

   Status remove(ServerContext* context, const RemoveRequest* request, RemoveReply* response) override {
     return Status::OK;
   }

   KeyValueStoreBackend* kv_store_backend_;
};

void RunServer() {
  std::string server_address("0.0.0.0:50001");
  KeyValueStoreImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Key value Store Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "First GLOG message " << std::endl;
  RunServer();
  return 0;
}
