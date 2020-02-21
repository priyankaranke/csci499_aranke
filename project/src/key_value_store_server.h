#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include <glog/logging.h>

#include "../protos/key_value_store.grpc.pb.h"
#include "key_value_store.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::ServerContext;
using grpc::Status;

using kvstore::KeyValueStore;
using kvstore::PutRequest;
using kvstore::PutReply;
using kvstore::GetRequest;
using kvstore::GetReply;
using kvstore::RemoveRequest;
using kvstore::RemoveReply;

class KeyValueStoreServer final : public KeyValueStore::Service {
 public:
   Status put(ServerContext* context, const PutRequest* request, PutReply* response) override;
   Status get(ServerContext* context, ServerReaderWriter<GetReply, GetRequest>* stream) override;
   Status remove(ServerContext* context, const RemoveRequest* request, RemoveReply* response) override;

 private:
   KvStore kv_store_;
};
