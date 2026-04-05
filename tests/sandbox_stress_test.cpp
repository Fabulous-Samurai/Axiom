// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "sandbox_proxy.h"

using namespace AXIOM::Sandbox;

int main() {
  std::cout << "--- [AXIOM] Sandbox & IPC Validation Start ---" << std::endl;

  auto& proxy = ProcessProxy::instance();

  // TEST 1: Normal Calculation via Sandbox
  std::cout << "[TEST 1] Sending normal command: '2 + 2 * 3'..." << std::endl;
  auto fut1 = proxy.execute("2 + 2 * 3");
  auto res1 = fut1.get();

  if (res1.success) {
    std::cout << "[SUCCESS] Result: " << res1.result
              << " (Time: " << res1.execution_time_ms << "ms)" << std::endl;
  } else {
    std::cout << "[FAILED] Error: " << res1.error << std::endl;
  }

  // TEST 2: Complex / Heavy Command (Simulated via long expression)
  std::cout << "\n[TEST 2] Sending complex symbolic structure..." << std::endl;
  // We'll send a command that the calculator might take time with or we just
  // test the IPC speed.
  auto fut2 = proxy.execute("sin(0.5) * cos(0.2) + tan(0.1)");
  auto res2 = fut2.get();

  if (res2.success) {
    std::cout << "[SUCCESS] Complex Result: " << res2.result << std::endl;
  }

  // TEST 3: REAL STALL & SELF-HEALING
  std::cout << "\n[TEST 3] Sending Poisonous 'STALL_INFINITE' (Timeout limit "
               "200ms)..."
            << std::endl;
  auto fut3 = proxy.execute("STALL_INFINITE", std::chrono::milliseconds(200));
  auto res3 = fut3.get();

  if (!res3.success && res3.error.find("Stalled") != std::string::npos) {
    std::cout << "[SUCCESS] Sentry/Proxy killed the hung process. Error: "
              << res3.error << std::endl;
  } else {
    std::cout << "[FAILED] Self-healing failed or not triggered." << std::endl;
  }

  // TEST 4: Recovery Validation
  std::cout << "\n[TEST 4] Validating recovery: Sending '10 + 10' to the new "
               "worker..."
            << std::endl;
  auto fut4 = proxy.execute("10 + 10");
  auto res4 = fut4.get();

  if (res4.success && res4.result.find("20") != std::string::npos) {
    std::cout << "[SUCCESS] System fully recovered and responsive!"
              << std::endl;
  } else {
    std::cout << "[FAILED] System failed to recover after stall." << std::endl;
  }

  std::cout << "\n--- [AXIOM] Validation Complete ---" << std::endl;
  return 0;
}
