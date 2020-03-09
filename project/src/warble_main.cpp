#include "func_client.h"
#include "warble.grpc.pb.h"

using warble::FollowReply;
using warble::FollowRequest;
using warble::ProfileReply;
using warble::ProfileRequest;
using warble::RegisteruserReply;
using warble::RegisteruserRequest;
using warble::Warble;
using warble::WarbleReply;
using warble::WarbleRequest;

const std::string kFuncClientPort = "localhost:50000";
const int kRegisteruserId = 1;
const int kWarbleId = 2;
const int kFollowId = 3;
const int kReadId = 4;
const int kProfileId = 5;

// method that hooks all the needed warble functions on initialization of
// FuncClient
void setup(FuncClient& func_client,
           const std::unordered_map<int, std::string>& function_map);

void profile(const std::string& username, FuncClient& func_client,
             int event_type);
void registeruser(const std::string& username, FuncClient& func_client,
                  int event_type);
void follow(const std::string& username, const std::string& to_follow,
            FuncClient& func_client, int event_type);
void warble(const std::string& username, std::string& text, int parent_id,
            FuncClient func_client, int event_type);

void prettyPrintWarble(WarbleReply warble_reply);

// Here is where the user's command line inputs will be interpreted
// and executed. Holds a FuncClient which talks to FuncServer
int main(int argc, char** argv) {
  FuncClient func_client(
      grpc::CreateChannel(kFuncClientPort, grpc::InsecureChannelCredentials()));

  // if we need to expose hook requests to warble, we can simply add them to
  // function_map as they come in
  std::unordered_map<int, std::string> function_map(
      {{kRegisteruserId, "registeruser"},
       {kWarbleId, "warble"},
       {kFollowId, "follow"},
       {kReadId, "read"},
       {kProfileId, "profile"}});

  setup(func_client, function_map);
  registeruser("priyank", func_client, kRegisteruserId);
  follow("priyank", "barath", func_client, kFollowId);
  follow("darth", "priyank", func_client, kFollowId);
  follow("barath", "darth", func_client, kFollowId);
  follow("tristan", "barath", func_client, kFollowId);

  profile("priyank", func_client, kProfileId);
  profile("barath", func_client, kProfileId);
  profile("darth", func_client, kProfileId);
  profile("tristan", func_client, kProfileId);

  return 0;
}

void setup(FuncClient& func_client,
           const std::unordered_map<int, std::string>& function_map) {
  for (auto it : function_map) {
    func_client.hook(it.first, it.second);
  }
}

void follow(const std::string& username, const std::string& to_follow,
            FuncClient& func_client, int event_type) {
  auto* sec_diff = new google::protobuf::Any();
  FollowRequest fr;
  fr.set_username(username);
  fr.set_to_follow(to_follow);
  sec_diff->PackFrom(fr);
  EventReply follow_event_reply = func_client.event(event_type, *sec_diff);
}

void registeruser(const std::string& username, FuncClient& func_client,
                  int event_type) {
  auto* any = new google::protobuf::Any();
  RegisteruserRequest rur;
  rur.set_username(username);
  any->PackFrom(rur);
  EventReply event_reply = func_client.event(event_type, *any);
}

void profile(const std::string& username, FuncClient& func_client,
             int event_type) {
  auto* any_other = new google::protobuf::Any();

  ProfileRequest pr;
  pr.set_username(username);
  any_other->PackFrom(pr);

  EventReply other_event_reply = func_client.event(event_type, *any_other);
  ProfileReply profile_reply;
  other_event_reply.payload().UnpackTo(&profile_reply);

  std::cout << "Followers of " << pr.username() << " are:" << std::endl;
  for (const std::string& f : profile_reply.followers()) {
    std::cout << f << std::endl;
  }

  std::cout << pr.username() << " is following:" << std::endl;
  for (const std::string& f : profile_reply.following()) {
    std::cout << f << std::endl;
  }
  std::cout << std::endl;
}

void warble(const std::string& username, std::string& text, int parent_id,
            FuncClient func_client, int event_type) {
  auto* any = new google::protobuf::Any();

  WarbleRequest request;
  request.set_username(username);
  request.set_text(text);
  request.set_parent_id(parent_id);
  any->PackFrom(request);

  EventReply event_reply = func_client.event(event_type, *any);
  WarbleReply reply;
  event_reply.payload().UnpackTo(&reply);

  prettyPrintWarble(reply);
}

void prettyPrintWarble(WarbleReply warble_reply) {
  Warble warble = warble_reply.warble();
  std::cout << warble.username() << ": " << warble.text() << " ("
            << warble.timestamp().seconds() << ")" << std::endl;
}