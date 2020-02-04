#include <iostream>
#include <grpcpp/grpcpp.h>
#include "backend_kv_store.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using kvstore::KeyValueStore;

// void RunServer(const std::string& db_path) {
//   std::string server_address("0.0.0.0:50001");
//   BackendKvStoreImpl service(db_path);

//   ServerBuilder builder;
//   builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
//   builder.RegisterService(&service);
//   std::unique_ptr<Server> server(builder.BuildAndStart());
//   std::cout << "Server listening on " << server_address << std::endl;
//   server->Wait();
// }

int main(int argc, char** argv) {
  std::cout << "Backend Key value store server is up" << std::endl;
  return 0;
}