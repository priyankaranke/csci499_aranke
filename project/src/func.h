#include <any>
#include <unordered_map>
#include <optional>

// General purpose Function as a Service that contains  
// logic of "func_server" to hook and unhook functions as well 
// as execute hooked functions for Warble
class Func {
 public:
   Func();
   void hook(const int event_type, const std::string &event_function);
   void unhook(const int event_type);

   // To use appropriately, for:
   // event_type 1 -> payload of type RegisterUserRequest is assumed
   // event_type 2 -> payload of type WarbleRequest is assumed
   // event_type 3 -> payload of type FollowRequest is assumed
   // event_type 4 -> payload of type ReadRequest is assumed
   // event_type 5 -> payload of type ProfileRequest is assumed
   //
   // if hooked to the event type, the appropriate function is then executed
   // and the appropriate Reply object is returned

   // TODO: check for valid event_type, payload pairs?
   std::optional<std::any> event(const int event_type, const std::any &payload) const;   
   
 private:
   // method that hooks all the needed warble functions on initialization of Func
   void setup();
   
   // map of event_type -> function to be executed for that ID
   std::unordered_map<int, std::string> function_map_;

   // TODO: Add a private KeyValueStoreServer here that Func talks to

   // Test against an instance of KvStore not KeyValueStoreServer 
   // (testing everything but GRPC)
};
