#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace expressway {

/**
 * @brief ShaderManager handles SPIR-V compilation and module lifecycle.
 * In Phase 7, it integrates with glslang or uses pre-baked .qsb files for QRhi.
 */
class ShaderManager {
public:
    static ShaderManager& instance() {
        static ShaderManager manager;
        return manager;
    }

    // Load a precompiled shader module
    VkShaderModule loadModule(VkDevice device, const std::string& path) {
        if (cache_.count(path)) return cache_[path];

        // Simplified: Reads binary SPIR-V file
        // Actual implementation will read from expressway/shaders/bin/*.spv
        return VK_NULL_HANDLE; 
    }

private:
    ShaderManager() = default;
    std::unordered_map<std::string, VkShaderModule> cache_;
};

} // namespace expressway
