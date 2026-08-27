#include "telemetry.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <iostream>
#include <mutex>

namespace AXIOM {

class SharedMemoryManager {
 public:
  static SharedMemoryManager& instance() {
    static SharedMemoryManager manager;
    return manager;
  }

  ~SharedMemoryManager() { cleanup(); }

  bool init(const std::string& name, size_t size) {
    if (buffer_) cleanup();
    name_ = name;
    size_ = size;

#ifdef _WIN32
    handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                 0, static_cast<DWORD>(size), name.c_str());

    if (!handle_) return false;

    buffer_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, size);

    if (!buffer_) {
      CloseHandle(handle_);
      handle_ = nullptr;
      return false;
    }
#else
    // POSIX implementation for Linux and macOS
    // Ensure name starts with '/' for POSIX compliance
    std::string posix_name = (name[0] == '/') ? name : "/" + name;

    fd_ = shm_open(posix_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd_ == -1) return false;

    // Set size
    if (ftruncate(fd_, size) == -1) {
      close(fd_);
      shm_unlink(posix_name.c_str());
      return false;
    }

    buffer_ = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (buffer_ == MAP_FAILED) {
      close(fd_);
      shm_unlink(posix_name.c_str());
      buffer_ = nullptr;
      return false;
    }
    posix_path_ = posix_name;
#endif
    std::cout << "[IPC] Shared Memory initialized: " << name << " ("
              << size / 1024 << " KB)" << std::endl;
    return true;
  }

  void cleanup() {
    if (!buffer_) return;

#ifdef _WIN32
    UnmapViewOfFile(buffer_);
    if (handle_) CloseHandle(handle_);
    handle_ = nullptr;
#else
    munmap(buffer_, size_);
    if (fd_ != -1) close(fd_);
    if (!posix_path_.empty()) shm_unlink(posix_path_.c_str());
    fd_ = -1;
#endif
    buffer_ = nullptr;
  }

  void* buffer() { return buffer_; }

 private:
  SharedMemoryManager() = default;
  void* buffer_ = nullptr;
  size_t size_ = 0;
  std::string name_;
#ifdef _WIN32
  HANDLE handle_ = nullptr;
#else
  int fd_ = -1;
  std::string posix_path_;
#endif
};

// Global Telemetry Implementation using Zero-Copy Shared Memory
void TelemetryScribe::log_throughput(double ops_per_sec) {
  void* buf = SharedMemoryManager::instance().buffer();
  if (buf) {
    // Simple Ring Buffer Header Update (Wait-free)
    auto* data = static_cast<double*>(buf);
    *data = ops_per_sec;  // Simplified for Phase 7 start
  }
}

bool TelemetryScribe::start(const std::string& name) {
  return SharedMemoryManager::instance().init(
      name, 1024 * 1024);  // 1MB shared buffer
}

double TelemetryScribe::read_throughput() {
  void* buf = SharedMemoryManager::instance().buffer();
  if (buf) {
    auto* data = static_cast<double*>(buf);
    return *data;
  }
  return 0.0;
}

void TelemetryScribe::shutdown() {
  // Shared memory is cleaned up by the manager's destructor or explicit close
  std::cout << "[IPC] Telemetry Scribe shutdown." << std::endl;
}

TelemetryScribe& TelemetryScribe::instance() {
  static TelemetryScribe scribe;
  return scribe;
}

}  // namespace AXIOM
