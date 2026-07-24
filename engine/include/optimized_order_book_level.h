// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once

#include <cstddef>
#include <cstdint>

struct OptimizedOrderBookLevel {
  alignas(32) char data[32];
};
