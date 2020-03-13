#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

// General purpose key value store that is not beholdent to
// Warble or FaaS layer. Supports concurrent access by multiple
// callers
class KvStore {
 public:
  bool put(const std::string &key, const std::string &value);
  std::optional<std::vector<std::string>> get(const std::string &key);
  void remove(const std::string &key);

 private:
  std::mutex mtx_;
  std::unordered_map<std::string, std::vector<std::string>> map_;
};
