#include "func.h"

#ifndef WARBLE_GRPC_PB_H
#define WARBLE_GRPC_PB_H
#include "warble.grpc.pb.h"
#endif

using warble::ProfileReply;
using warble::ProfileRequest;
using warble::RegisteruserReply;
using warble::RegisteruserRequest;

// Identifiers that specify which string is prepended to the key before adding
// to key value store
const std::string kUserFollowing = "user_following:";
const std::string kUserFollower = "user_follower:";

Func::Func()
    : kv_client_(grpc::CreateChannel(KV_CLIENT_PORT,
                                     grpc::InsecureChannelCredentials())) {}

void Func::hook(const EventType &event_type,
                const std::string &event_function) {
  mtx_.lock();
  function_map_[event_type] = event_function;
  std::cout << "Inside func hook hooked " << event_type << " to "
            << event_function << std::endl;
  mtx_.unlock();
}

void Func::unhook(const EventType &event_type) {
  mtx_.lock();
  function_map_.erase(event_type);
  std::cout << "Inside func hook unhooked " << event_type << std::endl;
  mtx_.unlock();
}

// // TODO: spawn new thread for every event call
std::unique_ptr<google::protobuf::Any> Func::event(
    const EventType event_type, const google::protobuf::Any &payload) {
  std::cout << " reached FUNC's event call successfully " << std::endl;
  std::cout << " Event_type is: " << event_type << std::endl;

  std::unordered_map<EventType, std::string>::const_iterator result =
      function_map_.find(event_type);

  // no suitable function found; either event_type bogus or function was
  // unhooked or function not hooked to event_type yet
  if (result == function_map_.end()) {
    std::cout << "Couldn't find hooked function" << std::endl;
    return {};
  }

  // found a hooked function for the request, execute it
  if (result->second == "registeruser") {
    std::cout << "Talking to kv_client_ regarding registeruserRequest"
              << std::endl;

    // Create the request
    RegisteruserRequest request;
    payload.UnpackTo(&request);
    std::string user_follower_entry = kUserFollower + request.username();
    std::string user_following_entry = kUserFollowing + request.username();

    // Put it into KVStore (registeruserRequest doesn't put followers or
    // following) into both user_follower and user_following subtables
    kv_client_.put(user_follower_entry, "");
    kv_client_.put(user_following_entry, "");

    RegisteruserReply response;
    auto any = std::make_unique<google::protobuf::Any>();
    any->PackFrom(response);
    return any;
  }

  if (result->second == "profile") {
    std::cout << "Talking to kv_client_ regarding profileRequest" << std::endl;

    // Create the request
    ProfileRequest request;
    payload.UnpackTo(&request);
    std::vector<GetReply> user_follower =
        kv_client_.get(kUserFollower + request.username());
    std::vector<GetReply> user_following =
        kv_client_.get(kUserFollowing + request.username());

    ProfileReply reply;
    for (GetReply get_reply : user_follower) {
      std::cout << "Adding to follower " << get_reply.value() << std::endl;
      reply.add_followers(get_reply.value());
    }

    for (GetReply get_reply : user_following) {
      std::cout << "Adding to following " << get_reply.value() << std::endl;
      reply.add_following(get_reply.value());
    }

    auto any_other = std::make_unique<google::protobuf::Any>();
    any_other->PackFrom(reply);
    return any_other;
  }

  if (result->second == "follow") {
    std::cout << "Talking to kv_client_ regarding followRequest" << std::endl;

    FollowRequest request;
    payload.UnpackTo(&request);

    kv_client_.put(request.to_follow());
    FollowReply response;
    auto any = std::make_unique<google::protobuf::Any>();
    any->PackFrom(response);
    return any;
  }
}
