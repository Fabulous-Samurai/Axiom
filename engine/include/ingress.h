// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file ingress.h
 * @brief Phase E: Kernel-Bypass Networking (Operation VARIANT SHIFT)
 *
 * Replaces dynamic polymorphism with static-variant dispatch for sub-nanosecond ingress.
 */

#pragma once

#include <string>
#include <variant>
#include <functional>
#include "arena_allocator.h"
#include "axiom_export.h"

namespace AXIOM {

struct IngressFrame {
    void* data;
    size_t length;
    uint64_t timestamp_rdtsc;
};

// --- Concrete Ingress Providers ---

class AF_XDPChannel {
public:
    bool start(HarmonicArena* target_arena) noexcept;
    void stop() noexcept;
    size_t poll(std::function<void(const IngressFrame&)> callback) noexcept;
};

class PosixUDPChannel {
public:
    bool start(HarmonicArena* target_arena) noexcept;
    void stop() noexcept;
    size_t poll(std::function<void(const IngressFrame&)> callback) noexcept;
};

class MockHFTChannel {
public:
    bool start(HarmonicArena* target_arena) noexcept;
    void stop() noexcept;
    size_t poll(std::function<void(const IngressFrame&)> callback) noexcept;
};

/**
 * @brief AnyIngressChannel: The master variant for Ingress VARIANT SHIFT.
 */
using AnyIngressChannel = std::variant<
    AF_XDPChannel,
    PosixUDPChannel,
    MockHFTChannel
>;

/**
 * @brief High-performance dispatcher for Ingress polling.
 * Eliminates VTable lookup in the critical data path.
 */
struct IngressDispatcher {
    static size_t poll(AnyIngressChannel& channel, std::function<void(const IngressFrame&)> callback) noexcept {
        return std::visit([&](auto& concrete_channel) {
            return concrete_channel.poll(callback);
        }, channel);
    }

    static bool start(AnyIngressChannel& channel, HarmonicArena* target_arena) noexcept {
        return std::visit([&](auto& concrete_channel) {
            return concrete_channel.start(target_arena);
        }, channel);
    }

    static void stop(AnyIngressChannel& channel) noexcept {
        std::visit([&](auto& concrete_channel) {
            concrete_channel.stop();
        }, channel);
    }
};

class AXIOM_EXPORT IngressFactory {
public:
    enum class ProviderType {
        PROV_AF_XDP,
        PROV_POSIX_UDP,
        PROV_MOCK_HFT_FEED
    };

    static AnyIngressChannel create(ProviderType type, const std::string& interface_name);
};

} // namespace AXIOM
