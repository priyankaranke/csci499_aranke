#include "test_func.h"

FuncTest::FuncTest() {};

FuncTest::~FuncTest() {};

void FuncTest::SetUp() {};

void FuncTest::TearDown() {};

// do not need to test non-existent event since we have 
// restricted that with the enums

// test running an event with no hooked function
TEST_F(FuncTest, TestEventFunction) {
  Func::EventType BAD_EVENT_TYPE = Func::EventType::Follow;
  std::optional<std::any> result = func.event(BAD_EVENT_TYPE, "test");
  EXPECT_EQ(result.has_value(), false); 
}

