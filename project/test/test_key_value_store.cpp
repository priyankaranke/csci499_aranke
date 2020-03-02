#include "test_key_value_store.h"

KeyValueStoreServerTest::KeyValueStoreServerTest(){};

KeyValueStoreServerTest::~KeyValueStoreServerTest(){};

void KeyValueStoreServerTest::SetUp(){};

void KeyValueStoreServerTest::TearDown(){};

// test insertion and retrieval of a key that isn't already in the map
TEST_F(KeyValueStoreServerTest, TestPutAndGetNew) {
  kv.put("a", "b");
  std::vector<std::string> result = (kv.get("a")).value();
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result.at(0), "b");
}

// test insertion and retrieval of a key that IS already in the map
TEST_F(KeyValueStoreServerTest, TestPutAndGetExisting) {
  kv.put("a", "b");
  kv.put("a", "c");
  std::vector<std::string> result = (kv.get("a")).value();

  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result.at(0), "b");
  EXPECT_EQ(result.at(1), "c");
}

// test get of a key that IS NOT in the map
TEST_F(KeyValueStoreServerTest, TestGetNonExisting) {
  std::optional<std::vector<std::string> > result = kv.get("non_existent_key");
  EXPECT_EQ(result.has_value(), false);
}

// test removal of key that exists
TEST_F(KeyValueStoreServerTest, TestRemoval) {
  kv.put("a", "b");
  kv.put("a", "c");
  kv.remove("a");

  std::optional<std::vector<std::string> > result = kv.get("a");
  EXPECT_EQ(result.has_value(), false);
}
