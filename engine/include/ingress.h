// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file ingress.h
 * @brief Phase E: Kernel-Bypass Networking (AF_XDP / DMA Ingress)
 * 
 * Defines the zero-copy ingress path for market-data and sensor inputs.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "arena_allocator.h"
#include "axiom_export.h"

namespace AXIOM {

/**
 * @brief Raw packet or data frame received via zero-copy path
 */
struct IngressFrame {
    void* data;
    size_t length;
    uint64_t timestamp_rdtsc;
};

/**
 * @brief Abstract interface for high-performance data ingress
 */
class AXIOM_EXPORT IngressChannel {
public:
    virtual ~IngressChannel() = default;

    /**
     * @brief Start receiving data into the provided arena
     */
    virtual bool start(HarmonicArena* target_arena) = 0;
    
    /**
     * @brief Stop ingress
     */
    virtual void stop() = 0;

    /**
     * @brief Poll for new data frames (0.25ns latency target)
     */
    virtual size_t poll(std::function<void(const IngressFrame&)> callback) = 0;
};

/**
 * @brief Factory and Registry for Ingress Providers (AF_XDP, DPDK, Mock)
 */
class AXIOM_EXPORT IngressFactory {
public:
    enum class ProviderType {
        PROV_AUTO,
        PROV_AF_XDP,
        PROV_POSIX_UDP,
        PROV_MOCK_HFT_FEED
    };

    static std::unique_ptr<IngressChannel> create(ProviderType type, const std::string& interface_name);
};

} // namespace AXIOM

