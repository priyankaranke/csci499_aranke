#include <iostream>
#include <grpcpp/grpcpp.h>
#include "backend_kv_store.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using kvstore::KeyValueStore;
using kvstore::PutRequest;
using kvstore::PutReply;

class BackendKvStoreImpl final : public KeyValueStore::Service {
    Status put(
        ServerContext* context, 
        const PutRequest* request, 
        PutReply* reply
    ) override {
        std::string key = request->key();
        std::string value = request->value();

        return Status::OK;
    } 
};

void RunServer() {
  std::string server_address("0.0.0.0:50001");
  BackendKvStoreImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on port: " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}