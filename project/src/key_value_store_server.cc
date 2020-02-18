#include <iostream>
#include "key_value_store_server.h"

typedef std::unordered_map<std::string, std::unique_ptr<std::vector<std::string>>> KvMap;

KeyValueStoreBackend::KeyValueStoreBackend() {
  map_ = KvMap();
}

// if we find the key, add value to vector of values (don't overwrite key->value pair)
// otherwise just add the key->value pair like a regular map
int KeyValueStoreBackend::put(const std::string key, const std::string value) {
  mtx_.lock();
  
  if (map_.find(key) == map_.end()) {
    map_[key] = std::unique_ptr<std::vector<std::string>>(new std::vector<std::string>());
  } 
  map_[key]->push_back(value);

  mtx_.unlock();
  // put was successful, return 0
  return 0;
}

std::vector<std::string> KeyValueStoreBackend::get(const std::string key) {
  mtx_.lock();

  KvMap::const_iterator result = map_.find(key);
  std::vector<std::string> result_vec;
  if (result != map_.end()) {
    result_vec = *(result->second);
  }

  mtx_.unlock();
  return result_vec;
}

void KeyValueStoreBackend::remove(const std::string key) {
  mtx_.lock();
  map_.erase(key);
  mtx_.unlock();
}
