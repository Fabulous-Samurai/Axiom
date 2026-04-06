#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

namespace expressway {

class VulkanContext {
public:
    static VulkanContext& instance() {
        static VulkanContext ctx;
        return ctx;
    }

    bool init() {
        if (initialized_) return true;

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "AXIOM Expressway";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Expressway Core";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
            std::cerr << "[Expressway] Failed to create Vulkan instance!" << std::endl;
            return false;
        }

        // Select physical device (simplified)
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
        if (deviceCount == 0) {
            std::cerr << "[Expressway] No Vulkan-supported GPUs found!" << std::endl;
            return false;
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
        physical_device_ = devices[0]; // Just take the first one for now

        // Create logical device
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = 0; // Simplified
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;

        if (vkCreateDevice(physical_device_, &deviceCreateInfo, nullptr, &device_) != VK_SUCCESS) {
            std::cerr << "[Expressway] Failed to create logical device!" << std::endl;
            return false;
        }

        initialized_ = true;
        std::cout << "[Expressway] Vulkan initialized successfully." << std::endl;
        return true;
    }

    void cleanup() {
        if (!initialized_) return;
        vkDestroyDevice(device_, nullptr);
        vkDestroyInstance(instance_, nullptr);
        initialized_ = false;
    }

private:
    VulkanContext() = default;
    ~VulkanContext() { cleanup(); }

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    bool initialized_ = false;
};

// Global entry point
extern "C" bool expressway_init() {
    return VulkanContext::instance().init();
}

} // namespace expressway
