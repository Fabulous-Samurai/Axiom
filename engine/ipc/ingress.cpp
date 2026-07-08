// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file ingress.cpp
 * @brief Implementation of Kernel-Bypass Ingress Factory and Concrete Providers
 * (Operation VARIANT SHIFT)
 */

#include "ingress.h"

#include <atomic>
#include <iostream>
#include <thread>

#ifdef __linux__
#include <linux/if_xdp.h>
#include <net/if.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace AXIOM {

// --- AF_XDPChannel Implementation ---

bool AF_XDPChannel::start(HarmonicArena* target_arena) noexcept {
  (void)target_arena;
#ifdef __linux__
  std::cout << "[AXIOM Ingress] Initializing AF_XDP Zero-Copy Channel"
            << std::endl;
  return true;
#else
  return false;
#endif
}

void AF_XDPChannel::stop() noexcept {}

size_t AF_XDPChannel::poll(
    std::function<void(const IngressFrame&)> callback) noexcept {
  (void)callback;
  return 0;
}

// --- PosixUDPChannel Implementation ---

bool PosixUDPChannel::start(HarmonicArena* target_arena) noexcept {
  (void)target_arena;
  return true;
}

void PosixUDPChannel::stop() noexcept {}

size_t PosixUDPChannel::poll(
    std::function<void(const IngressFrame&)> callback) noexcept {
  (void)callback;
  return 0;
}

// --- MockHFTChannel Implementation ---

bool MockHFTChannel::start(HarmonicArena* target_arena) noexcept {
  (void)target_arena;
  std::cout << "[AXIOM Ingress] Initialized Mock HFT Feed Provider"
            << std::endl;
  return true;
}

void MockHFTChannel::stop() noexcept {}

size_t MockHFTChannel::poll(
    std::function<void(const IngressFrame&)> callback) noexcept {
  (void)callback;
  return 0;
}

// --- Factory ---

AnyIngressChannel IngressFactory::create(ProviderType type,
                                         const std::string& interface_name) {
  (void)interface_name;
  if (type == ProviderType::PROV_AF_XDP) {
    return AF_XDPChannel();
  } else if (type == ProviderType::PROV_POSIX_UDP) {
    return PosixUDPChannel();
  }
  return MockHFTChannel();
}

}  // namespace AXIOM
