#include <iostream>

#include "fake_key_value_store.h"

void FakeKeyValueStoreClient::put(const std::string& key,
                                  const std::string& value) {
  if (map_.find(key) == map_.end()) {
    std::vector<std::string> new_vector;
    map_[key] = new_vector;
  }
  map_[key].push_back(value);
}

void FakeKeyValueStoreClient::remove(const std::string& key) {
  map_.erase(key);
}

const std::vector<GetReply> FakeKeyValueStoreClient::get(
    const std::string& key) const {
  std::vector<GetReply> get_replies;
  std::vector<std::string> response;

  std::unordered_map<std::string, std::vector<std::string>>::const_iterator
      result = map_.find(key);
  if (result != map_.end()) {
    response = result->second;
  }

  for (std::string& result : response) {
    GetReply reply;
    reply.set_value(result);
    get_replies.push_back(reply);
  }
  return get_replies;
}