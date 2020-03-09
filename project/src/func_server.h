#include <glog/logging.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "func.grpc.pb.h"
#include "func.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using func::EventReply;
using func::EventRequest;
using func::FuncService;
using func::HookReply;
using func::HookRequest;
using func::UnhookReply;
using func::UnhookRequest;

// Class that hooks and unhooks functions to event_types and executes them.
// Logic in class "func"
class FuncServer final : public FuncService::Service {
 public:
  // specify that a certain function is to be used to process a certain event_id
  Status hook(ServerContext* context, const HookRequest* request,
              HookReply* response) override;

  // specify that a certain function is to not longer be used to process a
  // certain event_id
  Status unhook(ServerContext* context, const UnhookRequest* request,
                UnhookReply* response) override;

  // process the incoming event_id based on the function previously hooked
  Status event(ServerContext* context, const EventRequest* request,
               EventReply* response) override;

 private:
  // Func performs the actual hooking, unhooking, event, maintenance of related
  // data structures
  Func func_;
};
