#include <cassert>

#include "axui/binding.h"

// Temel binding testi (detaylar Phase 3'te)
void placeholder_binding() {
  axui::BindingRegistry registry;
  double received = 0;
  registry.registerBinding("@test.value", [&](double v) { received = v; });
  registry.updateValue("@test.value", 42.0);
  assert(received == 42.0);
}
