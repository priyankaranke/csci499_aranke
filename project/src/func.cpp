#include <glog/logging.h>
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

Func::Func() { google::InitGoogleLogging(); }

void Func::hook(const EventType &event_type,
                const std::string &event_function) {
  mtx_.lock();
  function_map_[event_type] = event_function;
  LOG(INFO) << "Inside func hook hooked " << event_type << " to "
            << event_function << std::endl;
  mtx_.unlock();
}

void Func::unhook(const EventType &event_type) {
  mtx_.lock();
  function_map_.erase(event_type);
  LOG(INFO) << "Inside func hook unhooked " << event_type << std::endl;
  mtx_.unlock();
}

// // TODO: spawn new thread for every event call
google::protobuf::Any *Func::event(EventType event_type,
                                   google::protobuf::Any payload,
                                   grpc::Status &status, Database &kv_client_) {
  std::unordered_map<EventType, std::string>::const_iterator result =
      function_map_.find(event_type);

  // no suitable function found; either event_type bogus or function was
  // unhooked or function not hooked to event_type yet
  if (result == function_map_.end()) {
    LOG(WARNING) << "Function not found" << std::endl;
    return nullptr;
  }

  // found a hooked function for the request, execute it
  if (result->second == "registeruser" &&
      event_type == Func::EventType::RegisterUser) {
    RegisteruserReply response = registeruserEvent(kv_client_, payload, status);
    if (!status.ok()) {
      return nullptr;
    }
    auto *any = new google::protobuf::Any();
    any->PackFrom(response);
    return any;
  }
  if (result->second == "profile" && event_type == Func::EventType::Profile) {
    return profileEvent(kv_client_, payload, status);
  }
  if (result->second == "follow" && event_type == Func::EventType::Follow) {
    FollowReply response = followEvent(kv_client_, payload, status);
    if (!status.ok()) {
      return nullptr;
    }
    auto *any = new google::protobuf::Any();
    any->PackFrom(response);
    return any;
  }
  if (result->second == "warble" && event_type == Func::EventType::Warble) {
    return warbleEvent(kv_client_, payload, status);
  }
  if (result->second == "read" && event_type == Func::EventType::Read) {
    return readEvent(kv_client_, payload, status);
  }
  LOG(WARNING) << "Event type not found. Possible bad call by client"
               << std::endl;
  status = grpc::Status(grpc::StatusCode::NOT_FOUND,
                        "Error: Problem with eventType.");
  return nullptr;
}

RegisteruserReply Func::registeruserEvent(Database &kv_client_,
                                          google::protobuf::Any &payload,
                                          Status &status) {
  RegisteruserRequest request;
  payload.UnpackTo(&request);
  std::string user_follower_entry = kUserFollower + request.username();
  std::string user_following_entry = kUserFollowing + request.username();

  RegisteruserReply response;
  // if user already exists, do not add
  if (isInKv(user_follower_entry, kv_client_)) {
    status = grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                          "User you are trying to "
                          "register already exists");
    LOG(ERROR) << status.error_code() << ": " << status.error_message()
               << std::endl;
    return response;
  }
  // Put it into kv store into both user_follower and user_following subtables
  kv_client_.put(user_follower_entry, "");
  kv_client_.put(user_following_entry, "");

  return response;
}

FollowReply Func::followEvent(Database &kv_client_,
                              google::protobuf::Any &payload, Status &status) {
  FollowRequest request;
  payload.UnpackTo(&request);

  FollowReply response;
  // make sure both A and B are valid
  if (!isInKv(kUserFollower + request.username(), kv_client_)) {
    status = grpc::Status(grpc::StatusCode::NOT_FOUND, "User does not exist");
    return response;
  }
  if (!isInKv(kUserFollower + request.to_follow(), kv_client_)) {
    status = grpc::Status(grpc::StatusCode::NOT_FOUND,
                          "User you are trying to "
                          "follow does not exist");
    LOG(ERROR) << status.error_code() << ": " << status.error_message()
               << std::endl;
    return response;
  }
  // if A already following B, do not readd
  std::vector<GetReply> found_vec =
      kv_client_.get(kUserFollowing + request.username());
  for (GetReply get_reply : found_vec) {
    if (get_reply.value() == request.to_follow()) {
      status = grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "You are already "
                            "following this user");
      LOG(ERROR) << status.error_code() << ": " << status.error_message()
                 << std::endl;
      return response;
    }
  }

  // if (A) is to follow (B)
  // in the 'UserFollowing' subtable add A to the followers of B
  kv_client_.put(kUserFollowing + request.username(), request.to_follow());
  // in the 'UserFollower' subtable, add B to the followers of A
  kv_client_.put(kUserFollower + request.to_follow(), request.username());

  return response;
}

google::protobuf::Any *Func::warbleEvent(Database &kv_client_,
                                         google::protobuf::Any &payload,
                                         Status &status) {
  WarbleRequest request;
  payload.UnpackTo(&request);

  WarbleReply response;
  // check that both the parent Warble (if specified) and the user exist
  if (!isInKv(kUserFollowing + request.username(), kv_client_)) {
    status = grpc::Status(grpc::StatusCode::NOT_FOUND, "User does not exist");
    LOG(ERROR) << status.error_code() << ": " << status.error_message()
               << std::endl;
    return nullptr;
  }

  // -1 is the default parent for all Warbles
  if (!isInKv(kWarblePost + request.parent_id(), kv_client_) &&
      std::stoi(request.parent_id()) != -1) {
    status = grpc::Status(grpc::StatusCode::NOT_FOUND,
                          "Warble you are trying to reply to"
                          " does not exist");
    LOG(ERROR) << status.error_code() << ": " << status.error_message()
               << std::endl;
    return nullptr;
  }

  // add a Warble as its own parent (easier to retrieve thread) and an actual
  // parent (if specified)
  int parent_id = std::stoi(request.parent_id());
  kv_client_.put(kWarbleChildren + std::to_string(parent_id),
                 std::to_string(latest_warble_id));
  kv_client_.put(kWarbleChildren + std::to_string(latest_warble_id),
                 std::to_string(latest_warble_id));

  // add warble to kvstore
  response = buildWarbleReplyFromRequest(request, latest_warble_id);
  postWarble(response.warble(), kv_client_);
  latest_warble_id++;

  auto *any = new google::protobuf::Any();
  any->PackFrom(response);

  return any;
}

bool Func::isInKv(const std::string &check_string, Database &kv_client_) {
  std::vector<GetReply> found_vec = kv_client_.get(check_string);
  if (found_vec.size() != 0) {
    return true;
  } else {
    return false;
  }
}

void Func::postWarble(const warble::Warble &warb, Database &kv_client_) {
  std::string warble_string;
  warb.SerializeToString(&warble_string);
  kv_client_.put(kWarblePost + std::to_string(latest_warble_id), warble_string);
}

google::protobuf::Any *Func::readEvent(Database &kv_client_,
                                       google::protobuf::Any &payload,
                                       Status &status) {
  ReadRequest request;
  payload.UnpackTo(&request);

  ReadReply response;
  // check if thread id actually exists
  if (!isInKv(kWarblePost + request.warble_id(), kv_client_)) {
    status = grpc::Status(grpc::StatusCode::NOT_FOUND, "Warble does not exist");
    LOG(ERROR) << status.error_code() << ": " << status.error_message()
               << std::endl;
    return nullptr;
  }

  // get child warble ids
  int id = std::stoi(request.warble_id());
  std::unordered_set<int> warble_thread;
  retrieveThreadIds(id, warble_thread, kv_client_);

  // use that to get the actual warble
  for (int warble_id : warble_thread) {
    // should only return one but API specifies stream
    std::vector<GetReply> warble_lookup =
        kv_client_.get(kWarblePost + std::to_string(warble_id));
    if (warble_lookup.size() == 0) {
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

google::protobuf::Any *Func::profileEvent(Database &kv_client_,
                                          google::protobuf::Any &payload,
                                          Status &status) {
  ProfileRequest request;
  payload.UnpackTo(&request);

  // check that user exists
  if (!isInKv(kUserFollowing + request.username(), kv_client_)) {
    status = grpc::Status(grpc::StatusCode::NOT_FOUND, "User does not exist");
    LOG(ERROR) << status.error_code() << ": " << status.error_message()
               << std::endl;
    return nullptr;
  }

  // Get the profile info from kv store
  std::vector<GetReply> followers =
      kv_client_.get(kUserFollower + request.username());
  std::vector<GetReply> followings =
      kv_client_.get(kUserFollowing + request.username());

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
                             Database &kv_client_) {
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