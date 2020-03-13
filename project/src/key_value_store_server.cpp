#include "key_value_store_server.h"

Status KeyValueStoreServer::put(ServerContext* context,
                                const PutRequest* request, PutReply* response) {
  bool put_success = kv_store_.put(request->key(), request->value());
  if (put_success) {
    return Status::OK;
  } else {
    return Status::CANCELLED;
  }
}

Status KeyValueStoreServer::get(
    ServerContext* context, ServerReaderWriter<GetReply, GetRequest>* stream) {
  GetRequest request;
  while (stream->Read(&request)) {
    std::optional<std::vector<std::string>> response =
        kv_store_.get(request.key());

    if (!response.has_value()) {
      return Status::OK;
    } else {
      for (std::string& result : response.value()) {
        GetReply reply;
        reply.set_value(result);
        stream->Write(reply);
      }
    }
  }
  return Status::OK;
}

Status KeyValueStoreServer::remove(ServerContext* context,
                                   const RemoveRequest* request,
                                   RemoveReply* response) {
  kv_store_.remove(request->key());
  return Status::OK;
}
