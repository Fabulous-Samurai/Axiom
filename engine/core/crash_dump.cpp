// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file crash_dump.cpp
 * @brief Implementation of the mmap-backed CrashVault for Phase 3
 */

#include "crash_dump.h"

#include <cstring>
#include <iostream>

#include "cpu_optimization.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace AXIOM {

CrashVault& CrashVault::instance() {
  static CrashVault vault;
  return vault;
}

bool CrashVault::initialize(const std::string& path) {
  size_t total_size = sizeof(VaultHeader) + VAULT_SIZE;

#ifdef _WIN32
  HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                             OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) return false;

  // Ensure file size
  LARGE_INTEGER liSize;
  liSize.QuadPart = total_size;
  SetFilePointerEx(hFile, liSize, NULL, FILE_BEGIN);
  SetEndOfFile(hFile);

  HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0,
                                       (DWORD)total_size, NULL);
  if (!hMapping) {
    CloseHandle(hFile);
    return false;
  }

  mmap_base_ = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, total_size);
  if (!mmap_base_) {
    CloseHandle(hMapping);
    CloseHandle(hFile);
    return false;
  }

  file_handle_ = hFile;
  mapping_handle_ = hMapping;
#else
  int fd = open(path.c_str(), O_RDWR | O_CREAT, 0666);
  if (fd == -1) return false;

  if (ftruncate(fd, total_size) == -1) {
    close(fd);
    return false;
  }

  mmap_base_ =
      mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mmap_base_ == MAP_FAILED) {
    close(fd);
    return false;
  }
  fd_ = fd;
#endif

  mapped_size_ = total_size;
  header_ = static_cast<VaultHeader*>(mmap_base_);
  records_ = reinterpret_cast<CrashRecord*>(static_cast<char*>(mmap_base_) +
                                            sizeof(VaultHeader));

  // Initialize header if first time
  if (header_->magic_signature != 0x4158494F4D563330) {  // "AXIOMV30"
    header_->head.store(0);
    header_->tail.store(0);
    header_->magic_signature = 0x4158494F4D563330;
  }

  std::cout << "[AXIOM Phase 3] Crash Vault Initialized at: " << path << " ("
            << total_size / 1024 << " KB)" << std::endl;
  return true;
}

void CrashVault::record(uint32_t event_id, const char* msg) noexcept {
  if (!header_) return;

  // Use atomic fetch_add for multi-producer safety (though SPSC is intended,
  // we make it thread-safe for the crash log)
  uint64_t index = header_->head.fetch_add(1, std::memory_order_relaxed);

  CrashRecord& rec = records_[index % MAX_RECORDS];

  rec.timestamp_rdtsc = AXIOM_RDTSC();
  rec.event_id = event_id;
  // Simple thread ID retrieval
#ifdef _WIN32
  rec.thread_id = GetCurrentThreadId();
#else
  rec.thread_id = 0;  // Simplified for prototype
#endif

  if (msg) {
    // Use strnlen for safety to avoid reading past buffer if not
    // null-terminated
    size_t len = 0;
    while (len < sizeof(rec.message) - 1 && msg[len] != '\0') {
      len++;
    }
    std::memcpy(rec.message, msg, len);
    rec.message[len] = '\0';
  }

  // No msync() here! We rely on the OS page cache to flush.
  // Calling msync() would introduce the very OS Jitter we are avoiding.
}

CrashVault::~CrashVault() {
#ifdef _WIN32
  if (mmap_base_) UnmapViewOfFile(mmap_base_);
  if (mapping_handle_) CloseHandle(mapping_handle_);
  if (file_handle_) CloseHandle(file_handle_);
#else
  if (mmap_base_) munmap(mmap_base_, mapped_size_);
  if (fd_ != -1) close(fd_);
#endif
}

}  // namespace AXIOM
