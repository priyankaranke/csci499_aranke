#include <iostream>

#include "key_value_store.grpc.pb.h"

using kvstore::GetReply;

class Database {
 public:
  virtual void put(const std::string& key, const std::string& value) = 0;
  virtual void remove(const std::string& key) = 0;
  virtual std::vector<GetReply> get(const std::string& key) = 0;
};