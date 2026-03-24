#pragma once
#include "parser.h"
#include "resolver.h"
#include "binding.h"
#include <chrono>

namespace axui {

struct CompileResult {
    UINode root;
    bool success = false;
    std::vector<std::string> errors;
    double compile_time_ms = 0;
    size_t node_count = 0;
    size_t binding_count = 0;
};

class Compiler {
public:
    CompileResult compile(const std::string& axui_json);
    CompileResult compile(const std::string& axui_json, const std::string& theme_json);

private:
    Parser parser_;
    ThemeResolver resolver_;
    BindingRegistry bindings_;
};

} // namespace axui
