#include <sys/time.h>

#include "func.h"

using warble::FollowReply;
using warble::FollowRequest;
using warble::ProfileReply;
using warble::ProfileRequest;
using warble::RegisteruserReply;
using warble::RegisteruserRequest;
using warble::Timestamp;
using warble::Warble;
using warble::WarbleReply;
using warble::WarbleRequest;

// Identifiers that specify which string is prepended to the key before adding
// to key value store
const std::string kUserFollowing = "user_following:";
const std::string kUserFollower = "user_follower:";
const std::string kWarble = "warble:";

int latest_warble_id = 0;

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
      if (get_reply.value() != "") {
        std::cout << "Adding to follower " << get_reply.value() << std::endl;
        reply.add_followers(get_reply.value());
      }
    }

    for (GetReply get_reply : user_following) {
      if (get_reply.value() != "") {
        std::cout << "Adding to following " << get_reply.value() << std::endl;
        reply.add_following(get_reply.value());
      }
    }

    auto any_other = std::make_unique<google::protobuf::Any>();
    any_other->PackFrom(reply);
    return any_other;
  }

  if (result->second == "follow") {
    std::cout << "Talking to kv_client_ regarding followRequest" << std::endl;

    FollowRequest request;
    payload.UnpackTo(&request);

    // if (A) is to follow (B)
    // in the 'UserFollowing' subtable add A to the followers of B
    kv_client_.put(kUserFollowing + request.username(), request.to_follow());
    // in the 'UserFollower' subtable, add B to the followers of A
    kv_client_.put(kUserFollower + request.to_follow(), request.username());

    FollowReply response;
    auto any = std::make_unique<google::protobuf::Any>();
    any->PackFrom(response);
    return any;
  }

  if (result->second == "warble") {
    std::cout << "Talking to kv_client_ regarding warbleRequest" << std::endl;
    WarbleRequest request;
    payload.UnpackTo(&request);

    WarbleReply reply = buildWarbleReplyFromRequest(request, latest_warble_id);
    std::string warble_string;
    reply.warble().SerializeToString(&warble_string);
    kv_client_.put(kWarble + std::to_string(latest_warble_id), warble_string);

    latest_warble_id++;

    auto any = std::make_unique<google::protobuf::Any>();
    any->PackFrom(reply);
    return any;
  }
}

WarbleReply Func::buildWarbleReplyFromRequest(WarbleRequest &request, int id) {
  WarbleReply reply;
  timeval tv;
  gettimeofday(&tv, 0);

  reply.mutable_warble()->set_username(request.username());
  reply.mutable_warble()->set_text(request.text());
  reply.mutable_warble()->set_parent_id(request.parent_id());
  reply.mutable_warble()->set_id(std::to_string(id));
  reply.mutable_warble()->mutable_timestamp()->set_seconds(tv.tv_sec);
  reply.mutable_warble()->mutable_timestamp()->set_useconds(tv.tv_usec);

  return reply;
}