#pragma once

#include <vector>
#include <stdexcept>
#include <cstdint>

#ifdef _WIN32
    #include <windows.h>
    #include <bcrypt.h>
    #pragma comment(lib, "bcrypt.lib")
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    #include <stdlib.h>
    #include <unistd.h>
#elif defined(__linux__)
    #include <sys/random.h>
    #include <fcntl.h>
    #include <unistd.h>
#else
    #include <fcntl.h>
    #include <unistd.h>
#endif

namespace AXIOM {

/**
 * @brief Cross-platform Cryptographically Secure Pseudo-Random Number Generator (CSPRNG).
 * Satisfies SonarCloud S2245 by using OS-provided secure entropy sources.
 */
class SecureRandom {
public:
    static void bytes(void* buffer, size_t len) {
#ifdef _WIN32
        NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)buffer, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        if (status < 0) throw std::runtime_error("BCryptGenRandom failed");
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
        arc4random_buf(buffer, len);
#else
        // Try getrandom first (modern Linux)
        #ifdef __linux__
        ssize_t ret = getrandom(buffer, len, 0);
        if (ret == (ssize_t)len) return;
        #endif

        // Fallback to /dev/urandom
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) throw std::runtime_error("Failed to open /dev/urandom");
        
        size_t total_read = 0;
        uint8_t* ptr = static_cast<uint8_t*>(buffer);
        while (total_read < len) {
            ssize_t n = read(fd, ptr + total_read, len - total_read);
            if (n <= 0) {
                close(fd);
                throw std::runtime_error("Failed to read from /dev/urandom");
            }
            total_read += n;
        }
        close(fd);
#endif
    }

    template<typename T>
    static T generate() {
        T val;
        bytes(&val, sizeof(T));
        return val;
    }

    /**
     * @brief Generates a random value in range [min, max]
     */
    static int range(int min, int max) {
        if (min > max) return min;
        unsigned int r;
        bytes(&r, sizeof(r));
        return min + (int)(r % (unsigned int)(max - min + 1));
    }

    /**
     * @brief Generates a random double in range [0, 1]
     */
    static double uniform() {
        uint64_t r;
        bytes(&r, sizeof(r));
        // Use 53 bits for double precision
        return (double)(r & 0x1FFFFFFFFFFFFF) / (double)0x1FFFFFFFFFFFFF;
    }
};

} // namespace AXIOM
