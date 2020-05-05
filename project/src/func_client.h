#ifndef FUNC_CLIENT_H
#define FUNC_CLIENT_H

#include <grpcpp/grpcpp.h>
#include <iostream>

#include "func.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using func::EventReply;
using func::EventRequest;
using func::FuncService;
using func::HookReply;
using func::HookRequest;
using func::UnhookReply;
using func::UnhookRequest;

namespace function_constants {
const int kRegisteruserId = 1;
const int kWarbleId = 2;
const int kFollowId = 3;
const int kReadId = 4;
const int kProfileId = 5;
const int kStreamId = 6;
}  // namespace function_constants

class FuncClient {
 public:
  FuncClient() = delete;
  FuncClient(const FuncClient &) = delete;

  FuncClient(std::shared_ptr<Channel> channel);

  // Allows a particular event_function to be called when incoming event_type
  void hook(const int event_type, const std::string &event_function);

  // Removes the hooked event_function from attached event_type
  void unhook(const int event_type);

  // Executes the function associated and returns a Reply object as payload
  const EventReply event(const int event_type,
                         google::protobuf::Any &payload) const;

 private:
  std::unique_ptr<FuncService::Stub> stub_;
};

#endif
