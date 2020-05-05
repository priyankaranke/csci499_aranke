#include "func_server.h"
#include "key_value_store_client.h"

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
  KeyValueStoreClient kv_client_(
      grpc::CreateChannel(kKvClientPort, grpc::InsecureChannelCredentials()));

  google::protobuf::Any* func_response =
      func_.event(static_cast<Func::EventType>(request->event_type()),
                  request->payload(), status, kv_client_);

  // status changed and func_response marked nullptr if in error
  // could use std::optional instead
  if (!func_response) {
    LOG(WARNING) << "func_response was null, request might not have gotten "
                    "executed as expected"
                 << std::endl;
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
      {{Func::EventType::RegisterUser, "registeruser"},
       {Func::EventType::Warble, "warble"},
       {Func::EventType::Follow, "follow"},
       {Func::EventType::Read, "read"},
       {Func::EventType::Profile, "profile"},
       {Func::EventType::Stream, "stream"}});
  for (auto it : function_map) {
    func_.hook(static_cast<Func::EventType>(it.first), it.second);
  }
}