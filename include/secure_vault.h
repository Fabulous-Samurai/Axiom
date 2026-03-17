/**
 * @file secure_vault.h
 * @brief Phase H: Hardware Security Enclaves
 * 
 * Provides an interface for hardware-protected memory (SGX/TrustZone)
 * to encrypt/decrypt the Mantis state machine memory.
 */

#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include "arena_allocator.h"
#include "axiom_export.h"

namespace AXIOM {

/**
 * @brief Secure Vault for Enclave-protected State Machine
 */
class AXIOM_EXPORT SecureMantisVault {
    std::unique_ptr<std::byte[]> encrypted_storage_;
    size_t storage_size_{0};
    bool enclave_active_{false};

public:
    SecureMantisVault() = default;

    /**
     * @brief Initialize hardware enclave (SGX/TrustZone)
     */
    bool initialize_enclave();

    /**
     * @brief Seal the state machine into encrypted memory
     */
    bool seal(const void* src, size_t size);

    /**
     * @brief Unseal the state machine into a protected buffer
     */
    bool unseal(void* dst, size_t size);

    /**
     * @brief Perform a secure computation transition (Enclave ECALL)
     */
    void secure_transition();
};

} // namespace AXIOM

