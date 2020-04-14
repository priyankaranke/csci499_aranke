#include <gflags/gflags.h>
#include <glog/logging.h>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include "key_value_store_server.h"

DEFINE_string(store, "", "File to back the key value store");

KeyValueStoreServer* kv_server;

// Define the function to be called when ctrl-c (SIGINT) is sent to process
void signal_callback_handler(int signum) {
  kv_server->writeToFile();
  exit(signum);
}

void RunServer() {
  std::string server_address("0.0.0.0:50001");
  KeyValueStoreServer service(FLAGS_store);

  kv_server = &service;
  signal(SIGINT, signal_callback_handler);

  // set the latest warble ID as 0
  service.setup();

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Key value Store Server listening on " << server_address
            << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  LOG(INFO) << "Starting Key Value Store Server " << std::endl;
  RunServer();
  return 0;
}
