#include "func_client.h"
#include "warble.grpc.pb.h"

using warble::ProfileReply;
using warble::ProfileRequest;
using warble::RegisteruserReply;
using warble::RegisteruserRequest;

const std::string FUNC_CLIENT_PORT = "localhost:50000";

// method that hooks all the needed warble functions on initialization of
// FuncClient
void setup();

// Here is where the user's command line inputs will be interpreted
// and executed. This holds a FuncClient which talks to FuncServer
int main(int argc, char** argv) {
  FuncClient func_client(grpc::CreateChannel(
      FUNC_CLIENT_PORT, grpc::InsecureChannelCredentials()));

  // REGISTERUSER
  auto* any = new google::protobuf::Any();
  int sample_event_type = 0;
  std::string sample_event_function = "registeruser";
  func_client.hook(sample_event_type, sample_event_function);

  RegisteruserRequest rur;
  rur.set_username("priyank");
  any->PackFrom(rur);
  EventReply event_reply = func_client.event(sample_event_type, *any);

  // TEST RUNNING UNHOOKED FUNCTION
  auto* diff = new google::protobuf::Any();
  func_client.unhook(sample_event_type);
  event_reply = func_client.event(sample_event_type, *diff);

  // FOLLOW
  auto* sec_diff = new google::protobuf::Any();
  sample_event_type = 3;
  sample_event_function = "follow";
  func_client.hook(sample_event_type, sample_event_function);

  FollowRequest fr;
  fr.set_username("priyank");
  fr.to_follow("barath");
  sec_diff->PackFrom(fr);
  EventReply follow_event_reply =
      func_client.event(sample_event_type, *sec_diff);

  // PROFILEREQUEST
  auto* any_other = new google::protobuf::Any();
  sample_event_type = 5;
  sample_event_function = "profile";
  func_client.hook(sample_event_type, sample_event_function);

  ProfileRequest pr;
  pr.set_username("priyank");
  any_other->PackFrom(pr);
  EventReply other_event_reply =
      func_client.event(sample_event_type, *any_other);
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

  return 0;
}

void setup() {}