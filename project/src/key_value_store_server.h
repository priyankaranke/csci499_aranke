#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>

// General purpose key value store that is not beholdent to 
// Warble or FaaS layer. Supports concurrent access by multiple
// callers
class KeyValueStoreBackend {
 public:
   KeyValueStoreBackend();
   int put(const std::string key, const std::string value);
   std::vector<std::string> get(const std::string key);
   void remove(const std::string key);

 private:
   std::mutex mtx_;
   std::unordered_map<std::string, std::unique_ptr<std::vector<std::string>>> map_;
};
