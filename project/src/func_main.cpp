#include <iostream>
#include "func_server.h"

void RunServer() {
  std::string server_address("0.0.0.0:50000");
  FuncServer service;

  // perform hooking of all functions
  service.setup();

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Func Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Starting Func Server " << std::endl;
  RunServer();
  return 0;
}
