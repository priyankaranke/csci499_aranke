#ifndef FAKE_KEY_VALUE_STORE_H
#define FAKE_KEY_VALUE_STORE_H

#include <iostream>

#include "database.h"

class FakeKeyValueStoreClient : public Database {
 public:
  // Inserts key value pair (duplicate key is okay)
  void put(const std::string& key, const std::string& value) override;

  // Removes all values associated with key
  void remove(const std::string& key) override;

  // Returns all associated values for key
  const std::vector<GetReply> get(const std::string& key) const override;

  // Initializes the latest warble id in kv_store as 0
  void setup();

 private:
  std::unordered_map<std::string, std::vector<std::string>> map_;
};

#endif
