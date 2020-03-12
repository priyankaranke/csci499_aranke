#include "database.h"

class FakeKeyValueStore {
  void put(const std::string& key, const std::string& value);
  void remove(const std::string& key);
  std::vector<GetReply> get(const std::string& key);
};