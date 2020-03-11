#include <memory>
#include <mutex>
#include <unordered_map>

#ifndef WARBLE_GRPC_PB_H
#define WARBLE_GRPC_PB_H
#include "warble.grpc.pb.h"
#endif
#include "key_value_store_client.h"

// port Func's KeyValueStoreClient should connect to (where KeyValueStoreServer
// is listening on)
const std::string KV_CLIENT_PORT = "0.0.0.0:50001";

// General purpose Function as a Service that contains
// logic of "func_server" to hook and unhook functions as well
// as execute hooked functions for Warble
class Func {
 public:
  enum EventType {
    RegisterUser = 0,
    Warble = 1,
    Follow = 2,
    Read = 3,
    Profile = 4
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
  //
  // if hooked to the event type, the appropriate function is then executed
  // and the appropriate Reply object is returned

  // TODO: check for valid event_type, payload pairs?
  google::protobuf::Any *event(const EventType event_type,
                               google::protobuf::Any payload);

 private:
  // map of event_type -> function to be executed for that ID
  std::unordered_map<EventType, std::string> function_map_;
  std::mutex mtx_;

  KeyValueStoreClient kv_client_;

  warble::WarbleReply buildWarbleReplyFromRequest(
      warble::WarbleRequest &request, int id);

  void postWarble(const warble::Warble &warb, KeyValueStoreClient &kv_client_);

  void retrieveThreadIds(int id, std::unordered_set<int> &warble_thread,
                         KeyValueStoreClient &kv_client_);

  google::protobuf::Any *packProfileResponse(
      std::vector<kvstore::GetReply> &followers,
      std::vector<kvstore::GetReply> &followings);

  google::protobuf::Any *createAndPackReadResponse(
      std::unordered_set<int> &warble_thread, KeyValueStoreClient &kv_client_);
};
