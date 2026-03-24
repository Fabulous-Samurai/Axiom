// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once
#include <vector>
#include <memory>
#include <string_view>
#include <algorithm>
#include <cstring>

struct Block {
    std::unique_ptr<char[]> memory;
    size_t size;
    size_t used;
    Block(std::unique_ptr<char[]> mem, size_t s, size_t u) 
        : memory(std::move(mem)), size(s), used(u) {}
};

class Arena {
    std::vector<Block> blocks;
public:
    explicit Arena(size_t blockSize = 1024 * 64) { allocateBlock(blockSize); }
    ~Arena() = default;
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    
    void allocateBlock(size_t size) {
        auto mem = std::make_unique<char[]>(size);
        blocks.emplace_back(std::move(mem), size, 0);
    }
    void reset() { 
        if (!blocks.empty() && blocks[0].size >= 1024 * 64) {
            for (auto& block : blocks) block.used = 0;
        } else {
            blocks.clear();
            allocateBlock(1024 * 64);
        }
    }
    
    template <typename T, typename... Args>
    T* alloc(Args&&... args) {
        size_t sizeNeeded = sizeof(T); size_t align = alignof(T);
        Block* current = &blocks.back();
        auto currentPtr = (uintptr_t)(current->memory.get() + current->used);
        size_t padding = (align - (currentPtr % align)) % align;

        if (current->used + padding + sizeNeeded > current->size) {
            allocateBlock(std::max(current->size * 2, sizeNeeded + align));
            current = &blocks.back(); currentPtr = (uintptr_t)(current->memory.get()); padding = 0;
        }
        current->used += padding; void* ptr = current->memory.get() + current->used; current->used += sizeNeeded;
        return new (ptr) T(std::forward<Args>(args)...);
    }

    std::string_view allocString(std::string_view sv) {
        size_t len = sv.length(); Block* current = &blocks.back();
        if (current->used + len > current->size) { allocateBlock(std::max(current->size * 2, len)); current = &blocks.back(); }
        char* ptr = current->memory.get() + current->used; std::memcpy(ptr, sv.data(), len); current->used += len;
        return std::string_view(ptr, len);
    }
};
