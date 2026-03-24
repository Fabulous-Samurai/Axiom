#include "axui/binding.h"

namespace axui {

void BindingRegistry::registerBinding(const std::string& path, std::function<void(double)> callback) {
    bindings_[path].callbacks.push_back(std::move(callback));
}

void BindingRegistry::updateValue(const std::string& path, double value) {
    auto it = bindings_.find(path);
    if (it != bindings_.end()) {
        for (auto& cb : it->second.callbacks) {
            cb(value);
        }
    }
}

void BindingRegistry::extractBindings(UINode& root) {
    // Phase 3'te implement edilecek
}

} // namespace axui
