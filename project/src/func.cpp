#include <sys/time.h>

#include "func.h"

using warble::FollowReply;
using warble::FollowRequest;
using warble::ProfileReply;
using warble::ProfileRequest;
using warble::ReadReply;
using warble::ReadRequest;
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
const std::string kWarblePost = "warble_post:";
const std::string kWarbleChildren = "warble_children:";

// keeps track of id of warbles posted
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
google::protobuf::Any *Func::event(const EventType event_type,
                                   google::protobuf::Any payload) {
  std::unordered_map<EventType, std::string>::const_iterator result =
      function_map_.find(event_type);

  // no suitable function found; either event_type bogus or function was
  // unhooked or function not hooked to event_type yet
  if (result == function_map_.end()) {
    return nullptr;
  }

  // found a hooked function for the request, execute it
  if (result->second == "registeruser") {
    // Create the request
    RegisteruserRequest request;
    payload.UnpackTo(&request);
    std::string user_follower_entry = kUserFollower + request.username();
    std::string user_following_entry = kUserFollowing + request.username();

    // Put it into kv store into both user_follower and user_following subtables
    kv_client_.put(user_follower_entry, "");
    kv_client_.put(user_following_entry, "");

    // Pack in response
    RegisteruserReply response;
    auto *any = new google::protobuf::Any();
    any->PackFrom(response);
    return any;
  }

  if (result->second == "profile") {
    // Create the request
    ProfileRequest request;
    payload.UnpackTo(&request);

    // Get the profile info from kv store
    std::vector<GetReply> user_follower =
        kv_client_.get(kUserFollower + request.username());
    std::vector<GetReply> user_following =
        kv_client_.get(kUserFollowing + request.username());

    // Packing in response
    return packProfileResponse(user_follower, user_following);
  }

  if (result->second == "follow") {
    // Create the request
    FollowRequest request;
    payload.UnpackTo(&request);

    // if (A) is to follow (B)
    // in the 'UserFollowing' subtable add A to the followers of B
    kv_client_.put(kUserFollowing + request.username(), request.to_follow());
    // in the 'UserFollower' subtable, add B to the followers of A
    kv_client_.put(kUserFollower + request.to_follow(), request.username());

    // Create and pack in response
    FollowReply response;
    auto any = new google::protobuf::Any();
    any->PackFrom(response);
    return any;
  }

  if (result->second == "warble") {
    // Create the request
    WarbleRequest request;
    payload.UnpackTo(&request);

    // add a Warble as its own parent and an actual parent (if specified)
    int parent_id = std::stoi(request.parent_id());
    kv_client_.put(kWarbleChildren + std::to_string(parent_id),
                   std::to_string(latest_warble_id));
    kv_client_.put(kWarbleChildren + std::to_string(latest_warble_id),
                   std::to_string(latest_warble_id));

    // add warble to kvstore
    WarbleReply response =
        buildWarbleReplyFromRequest(request, latest_warble_id);
    postWarble(response.warble(), kv_client_);

    latest_warble_id++;

    auto any = new google::protobuf::Any();
    any->PackFrom(response);
    return any;
  }

  if (result->second == "read") {
    // Create the request
    ReadRequest request;
    payload.UnpackTo(&request);

    // get child warble ids
    int id = std::stoi(request.warble_id());
    std::unordered_set<int> warble_thread;
    retrieveThreadIds(id, warble_thread, kv_client_);

    // use that to get the actual warble
    return createAndPackReadResponse(warble_thread, kv_client_);
  }

  return nullptr;
}

void Func::postWarble(const warble::Warble &warb,
                      KeyValueStoreClient &kv_client_) {
  std::string warble_string;
  warb.SerializeToString(&warble_string);
  kv_client_.put(kWarblePost + std::to_string(latest_warble_id), warble_string);
}

google::protobuf::Any *Func::createAndPackReadResponse(
    std::unordered_set<int> &warble_thread, KeyValueStoreClient &kv_client_) {
  ReadReply response;

  for (int warble_id : warble_thread) {
    // should only return one but API specifies stream
    std::vector<GetReply> warble_lookup =
        kv_client_.get(kWarblePost + std::to_string(warble_id));
    if (warble_lookup.size() != 0) {
      continue;
    }
    for (GetReply get_reply : warble_lookup) {
      std::string data = get_reply.value();
      warble::Warble *my_warb = response.add_warbles();
      my_warb->ParseFromString(data);
    }
  }

  auto *any = new google::protobuf::Any();
  any->PackFrom(response);
  return any;
}

google::protobuf::Any *Func::packProfileResponse(
    std::vector<kvstore::GetReply> &followers,
    std::vector<kvstore::GetReply> &followings) {
  ProfileReply response;

  for (GetReply get_reply : followers) {
    if (get_reply.value() != "") {
      response.add_followers(get_reply.value());
    }
  }

  for (GetReply get_reply : followings) {
    if (get_reply.value() != "") {
      response.add_following(get_reply.value());
    }
  }

  auto *any = new google::protobuf::Any();
  any->PackFrom(response);
  return any;
}

void Func::retrieveThreadIds(int id, std::unordered_set<int> &warble_thread,
                             KeyValueStoreClient &kv_client_) {
  std::vector<GetReply> warble_children_lookup =
      kv_client_.get(kWarbleChildren + std::to_string(id));

  if (warble_children_lookup.size() == 0) {
    return;
  }

  for (GetReply get_reply : warble_children_lookup) {
    int child_id = std::stoi(get_reply.value());
    const bool is_in = (warble_thread.find(child_id) != warble_thread.end());
    if (!is_in) {
      warble_thread.insert(child_id);
      retrieveThreadIds(child_id, warble_thread, kv_client_);
    }
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