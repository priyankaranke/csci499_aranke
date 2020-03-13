#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>

#include "key_value_store.grpc.pb.h"

using kvstore::GetReply;

class Database {
 public:
  // ensures no instantiation
  virtual ~Database(){};
  virtual void put(const std::string& key, const std::string& value) = 0;
  virtual void remove(const std::string& key) = 0;
  const virtual std::vector<GetReply> get(const std::string& key) const = 0;
};

#endif
