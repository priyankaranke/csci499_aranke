#include "test_key_value_store_server.h"
#include "key_value_store_server.h"

KeyValueStoreServerTest::KeyValueStoreServerTest() {}

KeyValueStoreServerTest::~KeyValueStoreServerTest() {};

void KeyValueStoreServerTest::SetUp() {};

void KeyValueStoreServerTest::TearDown() {};

TEST(KeyValueStoreServerTest, TestTwo) {
  KeyValueStoreTemp kv = KeyValueStoreTemp();
  EXPECT_EQ(2, kv.giveMeTwo());
}