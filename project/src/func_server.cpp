#include "func_server.h"

Status FuncServer::hook(ServerContext* context, const HookRequest* request,
                        HookReply* response) {
  return Status::OK;
}

Status FuncServer::unhook(ServerContext* context, const UnhookRequest* request,
                          UnhookReply* response) {
  return Status::OK;
}

Status FuncServer::event(ServerContext* context, const EventRequest* request,
                         EventReply* response) {
  return Status::OK;
}