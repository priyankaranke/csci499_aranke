#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include <glog/logging.h>

#include "../protos/func.grpc.pb.h"
#include "func.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::ServerContext;
using grpc::Status;

using func::FuncService;
using func::HookRequest;
using func::HookReply;
using func::UnhookRequest;
using func::UnhookReply;
using func::EventRequest;
using func::EventReply;

// Class that hooks and unhooks functions to event_types and executes them. Logic in class "func" 
class FuncServer final : public FuncService::Service {
 public:
   Status hook(ServerContext* context, const HookRequest* request, HookReply* response) override;
   Status unhook(ServerContext* context, const UnhookRequest* request, UnhookReply* response) override;
   Status event(ServerContext* context, const EventRequest* request, EventReply* response) override;

 private:
   Func func_;
};
