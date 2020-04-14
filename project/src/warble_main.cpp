#include <gflags/gflags.h>
#include <glog/logging.h>
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

DEFINE_string(registeruser, "", "Username to register");
DEFINE_string(user, "", "Username");
DEFINE_string(warble, "", "Warble text");
DEFINE_int32(reply, -1, "Reply to Warble");
DEFINE_string(follow, "", "User to follow");
DEFINE_int32(read, -1, "Warble thread to read");
DEFINE_bool(profile, false, "Retrieve user profile");

const std::string kFuncClientPort = "localhost:50000";

// Return reply objects in case client wants to access them/handle their memory
ProfileReply profile(const std::string& username, FuncClient& func_client,
                     int event_type);
RegisteruserReply registeruser(const std::string& username,
                               FuncClient& func_client, int event_type);
FollowReply follow(const std::string& username, const std::string& to_follow,
                   FuncClient& func_client, int event_type);
WarbleReply warblePost(const std::string& username, const std::string& text,
                       int parent_id, FuncClient& func_client, int event_type);
ReadReply read(int warble_id, FuncClient& func_client, int event_type);

void prettyPrintWarble(const Warble& warble);
void printCorrectFlagCombos();
void renderWarbleThread(
    const std::unordered_map<int, std::vector<int>>& warble_thread,
    int warble_id, std::unordered_map<int, Warble>& warble_lookup,
    int num_call);

// Here is where the user's command line inputs will be interpreted
// and executed. Holds a FuncClient which talks to FuncServer
int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  FuncClient func_client(
      grpc::CreateChannel(kFuncClientPort, grpc::InsecureChannelCredentials()));

  // Command line flag checking does preliminary checks like making sure
  // fields are non-empty, that the right combinations of fields are specified
  // together, that user is logged in (except in the case of registeruser)
  // REGISTERUSER
  if (FLAGS_registeruser != "" && FLAGS_user == "" && FLAGS_warble == "" &&
      FLAGS_reply == -1 && FLAGS_follow == "" && FLAGS_read == -1 &&
      FLAGS_profile == false) {
    registeruser(FLAGS_registeruser, func_client, kRegisteruserId);
  }
  // (USER AND WARBLE) OR REPLY
  else if (FLAGS_registeruser == "" && FLAGS_user != "" && FLAGS_warble != "" &&
           FLAGS_follow == "" && FLAGS_read == -1 && FLAGS_profile == false) {
    warblePost(FLAGS_user, FLAGS_warble, FLAGS_reply, func_client, kWarbleId);
  }
  // (USER AND FOLLOW)
  else if (FLAGS_registeruser == "" && FLAGS_user != "" && FLAGS_warble == "" &&
           FLAGS_reply == -1 && FLAGS_follow != "" && FLAGS_read == -1 &&
           FLAGS_profile == false) {
    follow(FLAGS_user, FLAGS_follow, func_client, kFollowId);
  }
  // (USER AND READ)
  else if (FLAGS_registeruser == "" && FLAGS_user != "" && FLAGS_warble == "" &&
           FLAGS_reply == -1 && FLAGS_follow == "" && FLAGS_read != -1 &&
           FLAGS_profile == false) {
    read(FLAGS_read, func_client, kReadId);
  }
  // (USER AND PROFILE)
  else if (FLAGS_registeruser == "" && FLAGS_user != "" && FLAGS_warble == "" &&
           FLAGS_reply == -1 && FLAGS_follow == "" && FLAGS_read == -1 &&
           FLAGS_profile == true) {
    profile(FLAGS_user, func_client, kProfileId);
  }
  // bad flag combination
  else {
    printCorrectFlagCombos();
  }

  gflags::ShutDownCommandLineFlags();
  return 0;
}

// memory is allocated twice in the program: once on the server side for
// google::protobuf::any messages and once on the client side for the same.
// in both cases we use set_allocated, delegating the memory cleanup to
// the google protobuf library.
// we could instead copy the contents and manage the memory manually but
// since it is done once on the client side and once on the server side
// (no client is managing server allocated memory and vice versa), I
// made the decision to let protobuf handle it
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
  if (event_reply.has_payload()) {
    std::cout << "Registered user: " << username << std::endl;
    event_reply.payload().UnpackTo(&response);
  }
  std::cout << "\n";
  return response;
}

FollowReply follow(const std::string& username, const std::string& to_follow,
                   FuncClient& func_client, int event_type) {
  auto* any = new google::protobuf::Any();
  FollowRequest request;
  request.set_username(username);
  request.set_to_follow(to_follow);
  any->PackFrom(request);

  EventReply event_reply = func_client.event(event_type, *any);
  FollowReply response;

  if (event_reply.has_payload()) {
    std::cout << username << " successfully followed: " << to_follow
              << std::endl;
    event_reply.payload().UnpackTo(&response);
  }
  std::cout << "\n";
  return response;
}

// Return a warble thread
ReadReply read(int warble_id, FuncClient& func_client, int event_type) {
  auto* any = new google::protobuf::Any();
  ReadRequest request;
  request.set_warble_id(std::to_string(warble_id));
  any->PackFrom(request);

  EventReply event_reply = func_client.event(event_type, *any);
  ReadReply reply;

  if (event_reply.has_payload()) {
    // create a map of warble and child warbles
    std::unordered_map<int, std::vector<int>> warble_thread;
    std::unordered_map<int, Warble> warble_lookup;

    event_reply.payload().UnpackTo(&reply);
    // Generate ancestry tree
    for (const Warble& warble : reply.warbles()) {
      warble_lookup[std::stoi(warble.id())] = warble;
      if (std::stoi(warble.parent_id()) != -1) {
        warble_thread[std::stoi(warble.parent_id())].push_back(
            std::stoi(warble.id()));
      }
    }
    // Pretty print children recursively
    prettyPrintWarble(warble_lookup[warble_id]);
    renderWarbleThread(warble_thread, warble_id, warble_lookup, 0);
  }
  std::cout << "\n";
  return reply;
}

void renderWarbleThread(
    const std::unordered_map<int, std::vector<int>>& warble_thread,
    int warble_id, std::unordered_map<int, Warble>& warble_lookup,
    int num_call) {
  // keeps track of how many dashes to print
  num_call++;

  // look for child warbles
  std::unordered_map<int, std::vector<int>>::const_iterator it =
      warble_thread.find(warble_id);
  if (it == warble_thread.end()) {
    return;
  }

  // render their trees
  std::vector<int> children = it->second;
  for (int child_id : children) {
    Warble child_warble = warble_lookup[child_id];
    for (int i = 0; i < num_call; i++) {
      std::cout << "--";
    }
    prettyPrintWarble(child_warble);
    renderWarbleThread(warble_thread, child_id, warble_lookup, num_call);
  }
}

ProfileReply profile(const std::string& username, FuncClient& func_client,
                     int event_type) {
  auto* any = new google::protobuf::Any();
  ProfileRequest request;
  request.set_username(username);
  any->PackFrom(request);

  EventReply event_reply = func_client.event(event_type, *any);
  ProfileReply response;

  if (event_reply.has_payload()) {
    event_reply.payload().UnpackTo(&response);
    if (response.followers().size() == 0) {
      std::cout << request.username() << " has no followers (yet)."
                << std::endl;
    } else {
      std::cout << "Followers of " << request.username()
                << " are:" << std::endl;
      for (const std::string& f : response.followers()) {
        std::cout << f << std::endl;
      }
    }

    if (response.following().size() == 0) {
      std::cout << request.username() << " is not following anyone (yet)."
                << std::endl;
    } else {
      std::cout << request.username() << " is following:" << std::endl;
      for (const std::string& f : response.following()) {
        std::cout << f << std::endl;
      }
    }
    std::cout << std::endl;
  }
  return response;
}

WarbleReply warblePost(const std::string& username, const std::string& text,
                       int parent_id, FuncClient& func_client, int event_type) {
  auto* any = new google::protobuf::Any();

  WarbleRequest request;
  request.set_username(username);
  request.set_text(text);
  request.set_parent_id(std::to_string(parent_id));
  any->PackFrom(request);

  EventReply event_reply = func_client.event(event_type, *any);
  WarbleReply response;

  if (event_reply.has_payload()) {
    event_reply.payload().UnpackTo(&response);
    prettyPrintWarble(response.warble());
  }
  return response;
}

void printCorrectFlagCombos() {
  std::cout << "Invalid flag combination! Try one of the following flag "
               "combinations? "
            << std::endl;
  std::cout << "--registeruser <username>	Registers the given username"
            << std::endl;
  std::cout << "--user <username> --warble <warble text>	Logs in as the "
               "given username and creates a new warble with the given text"
            << std::endl;
  std::cout << "--user <username> --reply <reply warble id>	Indicates that "
               "the new warble is a reply to the given id"
            << std::endl;
  std::cout << "--user <username> --follow <username>		Starts "
               "following the given username"
            << std::endl;
  std::cout << "--user <username> --read <warble id>		Reads the "
               "warble thread starting at the given id"
            << std::endl;
  std::cout
      << "--user <username> --profile			Gets the user’s "
         "profile of following and followers"
      << std::endl;
}

void prettyPrintWarble(const Warble& warble) {
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
  std::cout << ")" << std::endl << std::endl;
}