#include "test_func.h"

FuncTest::FuncTest() {};

FuncTest::~FuncTest() {};

void FuncTest::SetUp() {};

void FuncTest::TearDown() {};

// test hooking and running a non-existent function
TEST_F(FuncTest, TestHookNew) {
  int BAD_EVENT_ID = 100000;
  std::optional<std::any> result = func.event(BAD_EVENT_ID, "test");
  EXPECT_EQ(result.has_value(), false); 
}

