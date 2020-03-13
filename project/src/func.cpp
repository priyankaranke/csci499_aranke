#include "func.h"

#ifndef WARBLE_GRPC_PB_H
#define WARBLE_GRPC_PB_H
#include "../protos/warble.grpc.pb.h"
#endif

using warble::RegisteruserReply;
using warble::RegisteruserRequest;

Func::Func() { setup(); }

void Func::setup() {}

void Func::hook(const EventType event_type, const std::string &event_function) {
  mtx_.lock();
  function_map_[event_type] = event_function;
  mtx_.unlock();
}

void Func::unhook(const EventType event_type) {
  mtx_.lock();
  function_map_.erase(event_type);
  mtx_.unlock();
}

// TODO: spawn new thread for every event call
std::unique_ptr<google::protobuf::Message> Func::event(
    const EventType event_type, const std::any &payload) const {
  std::unordered_map<EventType, std::string>::const_iterator result =
      function_map_.find(event_type);

  // no suitable function found; either event_type bogus or function was
  // unhooked found a hooked function for the request, execute it
  if (result == function_map_.end()) {
    return nullptr;
  }

  // found a hooked function for the request, execute it
  // TODO: add specific functionality for each event
  // if (result->second == "registeruserRequest") {
  // }
}
