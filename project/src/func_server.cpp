#include "func_server.h"

Status FuncServer::hook(ServerContext* context, const HookRequest* request,
                        HookReply* response) {
  func_.hook(static_cast<Func::EventType>(request->event_type()),
             request->event_function());
  return Status::OK;
}

Status FuncServer::unhook(ServerContext* context, const UnhookRequest* request,
                          UnhookReply* response) {
  func_.unhook(static_cast<Func::EventType>(request->event_type()));
  return Status::OK;
}

Status FuncServer::event(ServerContext* context, const EventRequest* request,
                         EventReply* response) {
  // Internally signal different errors
  grpc::Status status;

  google::protobuf::Any* func_response =
      func_.event(static_cast<Func::EventType>(request->event_type()),
                  request->payload(), status);

  if (!func_response) {
    return grpc::Status(status.error_code(), status.error_message());
  } else {
    response->set_allocated_payload(func_response);
    return Status::OK;
  }
}

void FuncServer::setup() {
  // if we need to expose hook requests to warble, we can simply add them to
  // function_map as they come in
  std::unordered_map<int, std::string> function_map(
      {{function_constants::kRegisteruserId, "registeruser"},
       {function_constants::kWarbleId, "warble"},
       {function_constants::kFollowId, "follow"},
       {function_constants::kReadId, "read"},
       {function_constants::kProfileId, "profile"}});
  for (auto it : function_map) {
    func_.hook(static_cast<Func::EventType>(it.first), it.second);
  }
}