#include "test_func.h"
#include "fake_key_value_store.h"

using warble::FollowReply;
using warble::FollowRequest;
using warble::ProfileReply;
using warble::ProfileRequest;
using warble::ReadReply;
using warble::ReadRequest;
using warble::RegisteruserReply;
using warble::RegisteruserRequest;
using warble::WarbleReply;
using warble::WarbleRequest;

// Identifiers that specify which string is prepended to the key before adding
// to key value store
const std::string kUserFollowing = "user_following:";
const std::string kUserFollower = "user_follower:";
const std::string kWarblePost = "warble_post:";
const std::string kWarbleChildren = "warble_children:";

FuncTest::FuncTest() {
  fake_kv = FakeKeyValueStoreClient();
  std::unordered_map<int, std::string> function_map(
      {{Func::EventType::RegisterUser, "registeruser"},
       {Func::EventType::Warble, "warble"},
       {Func::EventType::Follow, "follow"},
       {Func::EventType::Read, "read"},
       {Func::EventType::Profile, "profile"}});
  for (auto it : function_map) {
    func.hook(static_cast<Func::EventType>(it.first), it.second);
  }

  registerAllUsers("priyank");
  registerAllUsers("tristan");
  registerAllUsers("barath");
  registerAllUsers("darth");
}

FuncTest::~FuncTest(){};

void FuncTest::SetUp() {}

void FuncTest::registerAllUsers(const std::string& username) {
  Func::EventType event_type = Func::EventType::RegisterUser;
  grpc::Status status;

  auto* any = new google::protobuf::Any();
  RegisteruserRequest request;
  request.set_username(username);
  any->PackFrom(request);

  google::protobuf::Any* result = func.event(event_type, *any, status, fake_kv);
}

void FuncTest::TearDown(){};

// do not need to test non-existent event since we have
// restricted that with the enums

// also do not need to test for blank inputs since
// it is checked in command line arguments

TEST_F(FuncTest, TestPostingAndReadingWarbleWorks) {
  // Create and Post a Warble (id 0)
  grpc::Status status;
  Func::EventType event_type = Func::EventType::Warble;

  auto* post_any = new google::protobuf::Any();
  WarbleRequest setup_request;
  setup_request.set_username("priyank");
  setup_request.set_text("priyank's sample warble");
  setup_request.set_parent_id(std::to_string(-1));
  post_any->PackFrom(setup_request);

  google::protobuf::Any* setup_result =
      func.event(event_type, *post_any, status, fake_kv);

  // Create and Post a reply warble
  auto* reply_any = new google::protobuf::Any();
  WarbleRequest request;
  request.set_username("barath");
  request.set_text("barath's reply warble");
  request.set_parent_id(std::to_string(0));
  reply_any->PackFrom(request);

  google::protobuf::Any* result =
      func.event(event_type, *reply_any, status, fake_kv);

  // Check resulting warble
  WarbleReply warble_reply;
  result->UnpackTo(&warble_reply);
  EXPECT_EQ(warble_reply.warble().id(), "1");
  EXPECT_EQ(warble_reply.warble().username(), "barath");
  EXPECT_EQ(warble_reply.warble().text(), "barath's reply warble");
  EXPECT_EQ(warble_reply.warble().parent_id(), "0");

  // Read thread
  Func::EventType read_event = Func::EventType::Read;
  auto* read_any = new google::protobuf::Any();
  ReadRequest read_request;
  read_request.set_warble_id(std::to_string(0));
  read_any->PackFrom(read_request);

  google::protobuf::Any* read_result =
      func.event(read_event, *read_any, status, fake_kv);
  std::unordered_set<std::string> thread;
  ReadReply read_reply;
  read_result->UnpackTo(&read_reply);

  for (const warble::Warble& warb : read_reply.warbles()) {
    thread.insert(warb.id());
  }

  EXPECT_EQ((thread.find("0") != thread.end()), true);
  EXPECT_EQ((thread.find("1") != thread.end()), true);
}

TEST_F(FuncTest, RegisteringUserTwiceFails) {
  Func::EventType event_type = Func::EventType::RegisterUser;
  grpc::Status status;

  // prepare payload
  auto* any = new google::protobuf::Any();
  std::string username = "priyank";
  RegisteruserRequest request;
  request.set_username(username);
  any->PackFrom(request);

  google::protobuf::Any* result = func.event(event_type, *any, status, fake_kv);
  EXPECT_EQ(status.error_code(), 6);
  EXPECT_EQ(status.error_message(),
            "User you are trying to register already exists");
}

TEST_F(FuncTest, TestFollowingUserWorks) {
  Func::EventType event_type = Func::EventType::Follow;
  grpc::Status status;

  // prepare payload
  auto* any = new google::protobuf::Any();
  FollowRequest request;

  std::string username = "priyank";
  std::string to_follow = "barath";
  request.set_username(username);
  request.set_to_follow(to_follow);
  any->PackFrom(request);

  google::protobuf::Any* result = func.event(event_type, *any, status, fake_kv);
  std::unordered_set<std::string> s_follower;
  std::vector<GetReply> kv_response;

  kv_response = fake_kv.get(kUserFollowing + username);
  for (GetReply get_reply : kv_response) {
    s_follower.insert(get_reply.value());
  }
  EXPECT_EQ((s_follower.find("barath") != s_follower.end()), true);

  kv_response = fake_kv.get(kUserFollower + to_follow);
  std::unordered_set<std::string> s_followed;
  for (GetReply get_reply : kv_response) {
    s_followed.insert(get_reply.value());
  }

  EXPECT_EQ((s_followed.find("priyank") != s_followed.end()), true);
}

TEST_F(FuncTest, TestProfilePopulatesAfterAFollow) {
  grpc::Status status;

  // Check that profile is blank at first
  auto* pre_any = new google::protobuf::Any();
  ProfileRequest pre_request;
  pre_request.set_username("priyank");
  pre_any->PackFrom(pre_request);

  Func::EventType pre_profile = Func::EventType::Profile;
  google::protobuf::Any* pre_result =
      func.event(pre_profile, *pre_any, status, fake_kv);
  ProfileReply pre_profile_reply;
  pre_result->UnpackTo(&pre_profile_reply);
  EXPECT_EQ(pre_profile_reply.followers().size(), 0);

  // Priyank follows barath
  auto* any = new google::protobuf::Any();
  Func::EventType follow_event = Func::EventType::Follow;
  FollowRequest request;
  std::string username = "priyank";
  std::string to_follow = "barath";
  request.set_username(username);
  request.set_to_follow(to_follow);
  any->PackFrom(request);
  google::protobuf::Any* result =
      func.event(follow_event, *any, status, fake_kv);

  // // Make sure change appears in both profiles
  auto* post_any = new google::protobuf::Any();

  ProfileRequest post_request;
  post_request.set_username("barath");
  post_any->PackFrom(post_request);

  Func::EventType post_profile = Func::EventType::Profile;
  google::protobuf::Any* post_result =
      func.event(post_profile, *post_any, status, fake_kv);

  ProfileReply post_profile_reply;
  post_result->UnpackTo(&post_profile_reply);
  EXPECT_EQ(post_profile_reply.followers().size(), 1);
  EXPECT_EQ(post_profile_reply.following().size(), 0);

  auto* other = new google::protobuf::Any();

  ProfileRequest post_request_other;
  post_request_other.set_username("priyank");
  other->PackFrom(post_request_other);

  Func::EventType post_profile_other = Func::EventType::Profile;
  google::protobuf::Any* post_result_other =
      func.event(post_profile_other, *other, status, fake_kv);

  ProfileReply post_profile_reply_other;
  post_result_other->UnpackTo(&post_profile_reply_other);
  EXPECT_EQ(post_profile_reply_other.followers().size(), 0);
  EXPECT_EQ(post_profile_reply_other.following().size(), 1);
}

TEST_F(FuncTest, TestFollowWithNonExistentUserFails) {
  Func::EventType event_type = Func::EventType::Follow;
  grpc::Status status;

  // prepare payload
  auto* any = new google::protobuf::Any();
  std::string username = "nonexistent";
  FollowRequest request;
  request.set_username(username);
  any->PackFrom(request);

  google::protobuf::Any* result = func.event(event_type, *any, status, fake_kv);
  EXPECT_EQ(status.error_code(), 5);
  EXPECT_EQ(status.error_message(), "User does not exist");
}

TEST_F(FuncTest, TestFollowingANonExistentUserFails) {
  Func::EventType event_type = Func::EventType::Follow;
  grpc::Status status;

  // prepare payload
  auto* any = new google::protobuf::Any();
  std::string username = "priyank";
  std::string to_follow = "nonexistent";

  FollowRequest request;
  request.set_username(username);
  request.set_to_follow(to_follow);
  any->PackFrom(request);

  google::protobuf::Any* result = func.event(event_type, *any, status, fake_kv);
  EXPECT_EQ(status.error_code(), 5);
  EXPECT_EQ(status.error_message(),
            "User you are trying to follow does not exist");
}

TEST_F(FuncTest, TestProfileWithNonExistentUserFails) {
  Func::EventType event_type = Func::EventType::Profile;
  grpc::Status status;

  // prepare payload
  auto* any = new google::protobuf::Any();
  std::string username = "nonexistent";
  ProfileRequest request;
  request.set_username(username);
  any->PackFrom(request);

  google::protobuf::Any* result = func.event(event_type, *any, status, fake_kv);
  EXPECT_EQ(status.error_code(), 5);
  EXPECT_EQ(status.error_message(), "User does not exist");
}

TEST_F(FuncTest, TestReadWithNonExistentIdFails) {
  Func::EventType event_type = Func::EventType::Read;
  grpc::Status status;

  // prepare payload
  auto* any = new google::protobuf::Any();
  std::string id = "1000";
  ReadRequest request;
  request.set_warble_id(id);
  any->PackFrom(request);

  google::protobuf::Any* result = func.event(event_type, *any, status, fake_kv);
  EXPECT_EQ(status.error_code(), 5);
  EXPECT_EQ(status.error_message(), "Warble does not exist");
}

TEST_F(FuncTest, TestWarbleWithNonExistentUserFails) {
  Func::EventType event_type = Func::EventType::Warble;
  grpc::Status status;

  // prepare payload
  auto* any = new google::protobuf::Any();
  std::string username = "nonexistent";
  WarbleRequest request;
  request.set_username(username);
  any->PackFrom(request);

  google::protobuf::Any* result = func.event(event_type, *any, status, fake_kv);
  EXPECT_EQ(status.error_code(), 5);
  EXPECT_EQ(status.error_message(), "User does not exist");
}

TEST_F(FuncTest, TestReplyingToNonexistentWarbleIdFails) {
  Func::EventType event_type = Func::EventType::Warble;
  grpc::Status status;

  // prepare payload
  auto* any = new google::protobuf::Any();
  std::string username = "priyank";
  WarbleRequest request;
  request.set_username(username);
  request.set_text("priyank's test warble");
  request.set_parent_id(std::to_string(100));
  any->PackFrom(request);

  google::protobuf::Any* result = func.event(event_type, *any, status, fake_kv);
  EXPECT_EQ(status.error_code(), 5);
  EXPECT_EQ(status.error_message(),
            "Warble you are trying to reply to does not exist");
}