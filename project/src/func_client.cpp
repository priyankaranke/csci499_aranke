#include "func_client.h"
#include "warble.grpc.pb.h"

FuncClient::FuncClient(std::shared_ptr<Channel> channel)
    : stub_(FuncService::NewStub(channel)) {}

void FuncClient::hook(const int event_type, const std::string &event_function) {
  // Data we are sending to the server.
  HookRequest request;
  request.set_event_type(event_type);
  request.set_event_function(event_function);

  // Container for the data we expect from the server.
  HookReply reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // The actual RPC.
  Status status = stub_->hook(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    std::cout << "HookReply on HOOK was returned to FuncClient with Status::OK"
              << std::endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}

void FuncClient::unhook(const int event_type) {
  // Data we are sending to the server.
  UnhookRequest request;
  request.set_event_type(event_type);

  // Container for the data we expect from the server.
  UnhookReply reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // The actual RPC.
  Status status = stub_->unhook(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    std::cout
        << "UnhookReply on UNHOOK was returned to FuncClient with Status::OK"
        << std::endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}

EventReply FuncClient::event(const int event_type,
                             google::protobuf::Any &payload) {
  // Data we are sending to the server.
  EventRequest request;
  request.set_event_type(event_type);
  request.set_allocated_payload(&payload);

  // Container for the data we expect from the server.
  EventReply reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  std::cout << "Sending rpc for Event from func_client " << std::endl;
  // The actual RPC.
  Status status = stub_->event(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    std::cout << "Event was returned to FuncClient with Status::OK"
              << std::endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    std::cout << "Error: " << status.error_message() << std::endl;
  }
  return reply;
}
