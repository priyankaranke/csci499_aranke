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
  google::protobuf::Any* func_response = func_.event(
      static_cast<Func::EventType>(request->event_type()), request->payload());

  if (!func_response) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND,
                        "StatusCode::NOT_FOUND: Function was likely unhooked "
                        "before being called");
  } else {
    response->set_allocated_payload(func_response);
    return Status::OK;
  }
}