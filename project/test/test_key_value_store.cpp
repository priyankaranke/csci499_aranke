#include <fstream>

#include "test_key_value_store.h"

KeyValueStoreServerTest::KeyValueStoreServerTest() : kv(filename) {}

KeyValueStoreServerTest::~KeyValueStoreServerTest(){};

void KeyValueStoreServerTest::SetUp(){};

void KeyValueStoreServerTest::TearDown(){};

// the following three tests implicitly test KvStore with empty string
// as constructor and the other constructors are tested below
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

// first writes contents of map to file test.txt, then reads from it
// testing both the writeToFile and the constructor
TEST_F(KeyValueStoreServerTest, TestPopulateMapFromGoodFile) {
  std::ofstream outfile("test.txt");

  KvStore* kv_persist = new KvStore("test.txt");
  kv_persist->put("a", "b");
  kv_persist->put("a", "c");
  kv_persist->put("d", "e");

  // tests writing to file
  kv_persist->writeToFile();

  // tests reading from file
  KvStore* kv_persist_read = new KvStore("test.txt");

  std::vector<std::string> result = (kv_persist_read->get("a")).value();
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result.at(0), "b");
  EXPECT_EQ(result.at(1), "c");

  result = (kv_persist_read->get("d")).value();
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result.at(0), "e");

  delete kv_persist;
  delete kv_persist_read;
}