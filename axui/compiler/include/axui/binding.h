#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

#include "node.h"

namespace axui {

class BindingRegistry {
 public:
  void registerBinding(const std::string& path,
                       std::function<void(double)> callback);
  void updateValue(const std::string& path, double value);
  void extractBindings(UINode& root);
  size_t count() const { return bindings_.size(); }

 private:
  struct Entry {
    std::vector<std::function<void(double)>> callbacks;
  };
  std::unordered_map<std::string, Entry> bindings_;
};

}  // namespace axui
