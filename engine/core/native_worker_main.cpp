// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file native_worker_main.cpp
 * @brief Isolated Axiom Sandbox Worker with Lock-Free IPC.
 */

#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "dynamic_calc.h"
#include "sandbox_ipc.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

int main() {
  std::string shm_name = "Local\\AXIOM_SANDBOX_SHM";  // Fixed for now

  AXIOM::Sandbox::SandboxIPCLayout* ipc = nullptr;

#ifdef _WIN32
  HANDLE h_map = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shm_name.c_str());
  if (!h_map) return 1;

  ipc = (AXIOM::Sandbox::SandboxIPCLayout*)MapViewOfFile(
      h_map, FILE_MAP_ALL_ACCESS, 0, 0,
      sizeof(AXIOM::Sandbox::SandboxIPCLayout));
  if (!ipc) {
    CloseHandle(h_map);
    return 1;
  }
#endif

  AXIOM::DynamicCalc calculator;

  // Worker Loop
  while (true) {
    // 1. Signal Liveness (Heartbeat)
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    ipc->worker_heartbeat.store(now, std::memory_order_relaxed);

    // 2. Poll for Requests (Lock-Free)
    AXIOM::Sandbox::SandboxRequest req;
    if (ipc->req_queue.pop(req)) {
      auto t0 = std::chrono::high_resolution_clock::now();
      AXIOM::Sandbox::SandboxResponse res;
      res.request_id = req.request_id;
      res.success = false;

      if (std::string_view(req.command.data()) == "STALL_INFINITE") {
        // Simulation of a 'Poison' command that hangs the process
        while (true) {
          std::this_thread::yield();
        }
      }
      auto calc_result = calculator.Evaluate(req.command.data());
      if (!calc_result.HasErrors()) {
        res.success = true;
        auto val = calc_result.GetDouble();
        if (val)
          std::snprintf(res.result.data(), res.result.size(), "%f", *val);
        else
          std::snprintf(res.result.data(), res.result.size(), "%s", "OK");
      } else {
        std::snprintf(res.error.data(), res.error.size(), "%s",
                      "Calculation Error");
      }

      auto t1 = std::chrono::high_resolution_clock::now();
      res.execution_time_ms =
          std::chrono::duration<double, std::milli>(t1 - t0).count();

      // 3. Push Result back (Lock-Free)
      while (!ipc->res_queue.push(res)) {
        std::this_thread::yield();  // Wait if response queue is full
      }
    }

    std::this_thread::sleep_for(
        std::chrono::microseconds(100));  // Adaptive polling
  }

#ifdef _WIN32
  UnmapViewOfFile(ipc);
  CloseHandle(h_map);
#endif
  return 0;
}
