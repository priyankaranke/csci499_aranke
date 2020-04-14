#include "key_value_store.h"
#include <glog/logging.h>
#include <fstream>
#include <iostream>
#include <regex>

// ASCII 176 is a good delimiter since it cannot be input
// on a keyboard (before deploying our app, we should could prevent this
// character being entered by checking for it in warble_main should an attack
// happen)
const static std::string delimiter = std::string(1, char(176));

// populate map_ from file if file is given. Error if file is invalid
KvStore::KvStore(const std::string &file_str) : filename(file_str) {
  // no kv backing file given
  if (file_str == "") {
    return;
  }

  std::ifstream infile(filename);
  if (!infile.good()) {
    std::ofstream outfile(filename);
    outfile.close();
  }
  LOG(INFO) << "File good, loading map from file" << std::endl;

  std::stringstream sstr;
  sstr << infile.rdbuf();
  std::string file_contents = sstr.str();

  size_t pos = 0;
  std::string key;
  std::string value;
  while ((pos = file_contents.find(delimiter)) != std::string::npos) {
    key = file_contents.substr(0, pos);
    file_contents.erase(0, pos + delimiter.length());

    pos = file_contents.find(delimiter);
    value = file_contents.substr(0, pos);
    file_contents.erase(0, pos + delimiter.length());

    put(key, value);
  }
  infile.close();
}

// returns if no backing file specified, else and writes map_ to file
void KvStore::writeToFile() {
  if (filename == "") {
    LOG(INFO) << "No file to write to, shut down kv service" << std::endl;
    return;
  }

  std::ofstream ofile(filename);
  for (auto it : map_) {
    std::string key = it.first;
    std::vector<std::string> values = it.second;
    for (auto value : values) {
      ofile << key << delimiter << value << delimiter;
    }
  }
  ofile.close();

  LOG(INFO) << "Key value pairs written to " << filename << std::endl;
}

// if we find the key, add value to vector of values (don't overwrite key->value
// pair) otherwise just add the key->value pair like a regular map
bool KvStore::put(const std::string &key, const std::string &value) {
  mtx_.lock();

  if (map_.find(key) == map_.end()) {
    std::vector<std::string> new_vector;
    map_[key] = new_vector;
  }
  map_[key].push_back(value);
  mtx_.unlock();
  // put was successful
  return true;
}

std::optional<std::vector<std::string>> KvStore::get(const std::string &key) {
  mtx_.lock();

  std::unordered_map<std::string, std::vector<std::string>>::const_iterator
      result = map_.find(key);
  if (result != map_.end()) {
    mtx_.unlock();
    return result->second;
  }
  mtx_.unlock();
  return {};
}

void KvStore::remove(const std::string &key) {
  mtx_.lock();
  map_.erase(key);
  mtx_.unlock();
}
