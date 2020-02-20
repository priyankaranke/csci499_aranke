#include <iostream>
#include "key_value_store.h"

// if we find the key, add value to vector of values (don't overwrite key->value pair)
// otherwise just add the key->value pair like a regular map
bool KeyValueStoreBackend::put(const std::string key, const std::string value) {
  mtx_.lock();
  
  if (map_.find(key) == map_.end()) {
    std::vector<std::string> new_vector; 
    map_[key] = new_vector;
  } 
  map_[key].push_back(value);

  mtx_.unlock();
  // put was successful, return 0
  return true;
}

std::optional<std::vector<std::string>> KeyValueStoreBackend::get(const std::string key) {
  mtx_.lock();

  std::unordered_map<std::string, std::vector<std::string>>::const_iterator result = map_.find(key);
  if (result != map_.end()) {
    mtx_.unlock();
    return result->second;
  }

  mtx_.unlock();
  return {};
}

void KeyValueStoreBackend::remove(const std::string key) {
  mtx_.lock();
  map_.erase(key);
  mtx_.unlock();
}
