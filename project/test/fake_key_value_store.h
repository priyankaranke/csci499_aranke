#ifndef FAKE_KEY_VALUE_STORE_H
#define FAKE_KEY_VALUE_STORE_H

#include <iostream>

#include "database.h"

class FakeKeyValueStoreClient : public Database {
 public:
  void put(const std::string& key, const std::string& value) override;
  void remove(const std::string& key) override;
  const std::vector<GetReply> get(const std::string& key) const override;
  void setup();

 private:
  std::unordered_map<std::string, std::vector<std::string>> map_;
};

#endif
