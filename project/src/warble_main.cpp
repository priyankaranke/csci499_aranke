#include <gflags/gflags.h>
#include <time.h>

#include "func_client.h"
#include "warble.grpc.pb.h"

using warble::FollowReply;
using warble::FollowRequest;
using warble::ProfileReply;
using warble::ProfileRequest;
using warble::ReadReply;
using warble::ReadRequest;
using warble::RegisteruserReply;
using warble::RegisteruserRequest;
using warble::Warble;
using warble::WarbleReply;
using warble::WarbleRequest;

using function_constants::kFollowId;
using function_constants::kProfileId;
using function_constants::kReadId;
using function_constants::kRegisteruserId;
using function_constants::kWarbleId;

DEFINE_bool(verbose, false, "Display program name before message");
DEFINE_string(message, "Hello world!", "Message to print");

const std::string kFuncClientPort = "localhost:50000";

// method that hooks all the needed warble functions on initialization of
// FuncClient
void setup(FuncClient& func_client,
           const std::unordered_map<int, std::string>& function_map);

void profile(const std::string& username, FuncClient& func_client,
             int event_type);
RegisteruserReply registeruser(const std::string& username,
                               FuncClient& func_client, int event_type);
void follow(const std::string& username, const std::string& to_follow,
            FuncClient& func_client, int event_type);
void warblePost(const std::string& username, const std::string& text,
                int parent_id, FuncClient& func_client, int event_type);
void read(int warble_id, FuncClient& func_client, int event_type);

void prettyPrintWarble(Warble warble);

// Here is where the user's command line inputs will be interpreted
// and executed. Holds a FuncClient which talks to FuncServer
int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

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
  profile("nonexist", func_client, kProfileId);

  warblePost("priyank", "priyank's first warble", -1, func_client, kWarbleId);
  warblePost("barath", "barath's now on warble", 2, func_client, kWarbleId);
  warblePost("darth", "darth's reply to priyank", 0, func_client, kWarbleId);
  warblePost("priyank", "priyank responds to barath!", 2, func_client,
             kWarbleId);

  read(1, func_client, kReadId);
  read(0, func_client, kReadId);
  read(2, func_client, kReadId);

  std::cout << FLAGS_message << std::endl;
  gflags::ShutDownCommandLineFlags();
  return 0;
}

void setup(FuncClient& func_client,
           const std::unordered_map<int, std::string>& function_map) {
  for (auto it : function_map) {
    func_client.hook(it.first, it.second);
  }
}

RegisteruserReply registeruser(const std::string& username,
                               FuncClient& func_client, int event_type) {
  // no need to explicitly delete since we use set_allocated in func_client
  // which takes ownership of the pointer and deletes it when the message is
  // destroyed (https://stackoverflow.com/a/43195097)
  auto* any = new google::protobuf::Any();
  RegisteruserRequest request;
  request.set_username(username);
  any->PackFrom(request);

  EventReply event_reply = func_client.event(event_type, *any);
  RegisteruserReply response;

  return response;
}

void follow(const std::string& username, const std::string& to_follow,
            FuncClient& func_client, int event_type) {
  auto* any = new google::protobuf::Any();
  FollowRequest fr;
  fr.set_username(username);
  fr.set_to_follow(to_follow);
  any->PackFrom(fr);
  EventReply follow_event_reply = func_client.event(event_type, *any);
}

void read(int warble_id, FuncClient& func_client, int event_type) {
  auto* any = new google::protobuf::Any();
  ReadRequest request;
  request.set_warble_id(std::to_string(warble_id));
  any->PackFrom(request);

  EventReply event_reply = func_client.event(event_type, *any);
  ReadReply warble_reply;
  event_reply.payload().UnpackTo(&warble_reply);

  // Print all of its children
  for (const Warble& warble : warble_reply.warbles()) {
    prettyPrintWarble(warble);
  }
}

void profile(const std::string& username, FuncClient& func_client,
             int event_type) {
  auto* any = new google::protobuf::Any();

  ProfileRequest pr;
  pr.set_username(username);
  any->PackFrom(pr);

  EventReply other_event_reply = func_client.event(event_type, *any);
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

void warblePost(const std::string& username, const std::string& text,
                int parent_id, FuncClient& func_client, int event_type) {
  auto* any = new google::protobuf::Any();

  WarbleRequest request;
  request.set_username(username);
  request.set_text(text);
  request.set_parent_id(std::to_string(parent_id));
  any->PackFrom(request);

  EventReply event_reply = func_client.event(event_type, *any);
  WarbleReply reply;
  event_reply.payload().UnpackTo(&reply);

  prettyPrintWarble(reply.warble());
}

void prettyPrintWarble(Warble warble) {
  // Get time
  struct tm* tm;
  time_t t;

  long seconds = warble.timestamp().seconds();
  tm = localtime(&seconds);
  char time_buf[30];
  strftime(time_buf, 30, "%Y:%m:%d %H:%M:%S", tm);

  // Print warble info
  std::cout << warble.id() << " " << warble.username() << ": " << warble.text()
            << " (";
  printf("%s", time_buf);
  std::cout << ")" << std::endl;
}