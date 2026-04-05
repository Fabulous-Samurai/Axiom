// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once
#include <cstddef>
#include <cstdint>

#ifdef _WIN32
#ifdef AXIOM_VULKAN_EXPORTS
#define AXIOM_VULKAN_EXPORT __declspec(dllexport)
#else
#define AXIOM_VULKAN_EXPORT __declspec(dllimport)
#endif
#else
#define AXIOM_VULKAN_EXPORT __attribute__((visibility("default")))
#endif

namespace AXIOM {
class VulkanExpressway;
}

extern "C" {
AXIOM_VULKAN_EXPORT void* AxiomVulkan_CreateExpressway();
AXIOM_VULKAN_EXPORT bool AxiomVulkan_Initialize(void* instance);
}
