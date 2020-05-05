#include <grpcpp/grpcpp.h>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "database.h"
#include "key_value_store.grpc.pb.h"
#include "warble.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using kvstore::GetReply;
using kvstore::GetRequest;
using kvstore::KeyValueStore;
using kvstore::PutReply;
using kvstore::PutRequest;
using kvstore::RemoveReply;
using kvstore::RemoveRequest;

// port Func's KeyValueStoreClient should connect to (where KeyValueStoreServer
// is listening on)
const std::string kKvClientPort = "0.0.0.0:50001";

// General purpose Function as a Service that contains
// logic of "func_server" to hook and unhook functions as well
// as execute hooked functions for Warble
class Func {
 public:
  enum EventType {
    RegisterUser = 1,
    Warble = 2,
    Follow = 3,
    Read = 4,
    Profile = 5,
    Stream = 6
  };

  Func();
  void hook(const EventType &event_type, const std::string &event_function);
  void unhook(const EventType &event_type);

  // To use appropriately, for:
  // event_type 1 -> payload of type RegisterUserRequest is assumed
  // event_type 2 -> payload of type WarbleRequest is assumed
  // event_type 3 -> payload of type FollowRequest is assumed
  // event_type 4 -> payload of type ReadRequest is assumed
  // event_type 5 -> payload of type ProfileRequest is assumed
  // event_type 6 -> payload of type StreamRequest is assumed
  //
  // if hooked to the event type, the appropriate function is then executed
  // and the appropriate Reply object is returned
  google::protobuf::Any *event(EventType event_type,
                               google::protobuf::Any payload, Status &status,
                               Database &kv_client_);

 private:
  // map of event_type -> function to be executed for that ID
  std::unordered_map<EventType, std::string> function_map_;
  std::mutex mtx_;

  // retrieves latest warble added recorded in kv store
  int getLatestWarbleId(const Database &kv_client_) const;

  // updates latestWarbleId on every new warble
  void incrementLatestWarbleId(Database &kv_client_);

  warble::RegisteruserReply registeruserEvent(Database &kv_client_,
                                              google::protobuf::Any &payload,
                                              Status &status);

  warble::FollowReply followEvent(Database &kv_client_,
                                  google::protobuf::Any &payload,
                                  Status &status);

  google::protobuf::Any *warbleEvent(Database &kv_client_,
                                     google::protobuf::Any &payload,
                                     Status &status);

  bool isInKv(const std::string &check_string, Database &kv_client_);

  warble::WarbleReply buildWarbleReplyFromRequest(
      warble::WarbleRequest &request, int id);

  void postWarble(const warble::Warble &warb, Database &kv_client_,
                  int latest_warble_id);

  // parse all hash tags from warble text
  std::unordered_set<std::string> parseTagsFromWarbleText(const std::string& text);

  // update hashtag -> warble_id pair in the key value store
  void addHashtagToKvStore(warble::WarbleRequest &request, Database &kv_client_, int id);

  // recursively gets warble children
  void retrieveThreadIds(int id, std::unordered_set<int> &warble_thread,
                         Database &kv_client_);

  google::protobuf::Any *profileEvent(Database &kv_client_,
                                      google::protobuf::Any &payload,
                                      Status &status);

  google::protobuf::Any *readEvent(Database &kv_client_,
                                   google::protobuf::Any &payload,
                                   Status &status);

  google::protobuf::Any *streamEvent(Database &kv_client_, google::protobuf::Any &payload, Status &status);
};
