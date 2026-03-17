#pragma once

#include <cstddef>
#include <cstdint>

struct OptimizedOrderBookLevel {
    alignas(32) char data[32];
};
