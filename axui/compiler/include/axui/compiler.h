#pragma once
#include <chrono>

#include "arena_allocator.h"
#include "binding.h"
#include "parser.h"
#include "resolver.h"

namespace axui {

struct CompileResult {
  UINode* root = nullptr;
  bool success = false;
  std::vector<std::string> errors;
  double compile_time_ms = 0;
  size_t node_count = 0;
  size_t binding_count = 0;
};

class Compiler {
 public:
  Compiler()
      : arena_(1024 * 1024, true),
        allocator_(&arena_) {}  // 1MB arena for UI tree

  CompileResult compile(const std::string& axui_json);
  CompileResult compile(const std::string& axui_json,
                        const std::string& theme_json);

 private:
  Parser parser_;
  ThemeResolver resolver_;
  BindingRegistry bindings_;
  AXIOM::MemoryArena arena_;
  AXIOM::ArenaAllocator<UINode> allocator_;
};

}  // namespace axui
