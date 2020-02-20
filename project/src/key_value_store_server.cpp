#include "key_value_store_server.h"

Status KeyValueStoreServer::put(ServerContext* context, const PutRequest* request, PutReply* response) {
    return Status::OK;
}

Status KeyValueStoreServer::get(ServerContext* context, ServerReaderWriter<GetReply, GetRequest>* stream) {
    return Status::OK;
}

Status KeyValueStoreServer::remove(ServerContext* context, const RemoveRequest* request, RemoveReply* response) {
    return Status::OK;
}
