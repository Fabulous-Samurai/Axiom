#ifdef ENABLE_PYTHON_FFI

#include "python_repl.h"
#include <iostream>
#include <sstream>
#include <algorithm>

PythonREPL::PythonREPL(PythonEngine* engine) 
    : python_engine_(engine), session_active_(false), indent_level_(0) {
}

void PythonREPL::StartInteractiveSession() {
    session_active_ = true;
    ResetREPLState();
    
    // Initialize Python interactive environment
    python_engine_->ExecutePython("import sys");
    python_engine_->ExecutePython("import os");
    python_engine_->ExecutePython("sys.ps1 = '>>> '");
    python_engine_->ExecutePython("sys.ps2 = '... '");
    
    // Set up helpful REPL environment
    std::string setup_code = R"(
# REPL Helper functions
def help_vars():
    """List all user-defined variables"""
    user_vars = {k: v for k, v in globals().items() 
                 if not k.startswith('_') and k not in ['help_vars', 'help_modules']}
    for name, value in user_vars.items():
        print(f"{name}: {type(value).__name__} = {repr(value)}")

def help_modules():
    """List imported modules"""
    modules = [k for k in globals().keys() if hasattr(globals()[k], '__file__')]
    print("Imported modules:", ', '.join(modules))

print("Python FFI REPL - Type 'help_vars()' or 'help_modules()' for assistance")
    )";
    
    python_engine_->ExecutePython(setup_code);
}

void PythonREPL::StopInteractiveSession() {
    session_active_ = false;
    ClearMultiLineBuffer();
}

std::string PythonREPL::ExecuteInteractive(const std::string& input) {
    if (!session_active_) {
        return "Error: REPL session not active";
    }
    
    // Add to command history
    command_history_.push_back(input);
    
    // Handle special REPL commands
    if (input == "exit()" || input == "quit()") {
        StopInteractiveSession();
        return "Goodbye!";
    }
    
    // Check if this is multi-line input
    if (IsMultiLineInput(input)) {
        AddToMultiLineBuffer(input);
        if (NeedsMoreInput(input)) {
            return GeneratePrompt(); // Continue multi-line input
        } else {
            return ExecuteMultiLineBuffer();
        }
    }
    
    // Execute single-line input
    auto result = python_engine_->EvaluatePython(input);
    if (result.result.has_value()) {
        auto& res = result.result.value();
        if (std::holds_alternative<std::string>(res)) {
            last_output_ = std::get<std::string>(res);
            return FormatOutput(last_output_);
        }
    }
    
    if (result.error.has_value()) {
        return FormatOutput("Error: Operation failed", true);
    }
    
    return "";
}

bool PythonREPL::IsMultiLineInput(const std::string& input) {
    // Check for constructs that typically require multiple lines
    std::string trimmed = input;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    
    return trimmed.find("def ") == 0 ||
           trimmed.find("class ") == 0 ||
           trimmed.find("if ") == 0 ||
           trimmed.find("elif ") == 0 ||
           trimmed.find("else:") == 0 ||
           trimmed.find("for ") == 0 ||
           trimmed.find("while ") == 0 ||
           trimmed.find("try:") == 0 ||
           trimmed.find("except") == 0 ||
           trimmed.find("finally:") == 0 ||
           trimmed.find("with ") == 0 ||
           input.back() == ':';
}

void PythonREPL::AddToMultiLineBuffer(const std::string& input) {
    multiline_buffer_.push_back(input);
    
    // Update indent level based on input
    if (input.back() == ':') {
        indent_level_++;
    } else if (input.empty() && indent_level_ > 0) {
        indent_level_--;
    }
}

std::string PythonREPL::ExecuteMultiLineBuffer() {
    if (multiline_buffer_.empty()) {
        return "";
    }
    
    // Join multi-line buffer
    std::stringstream code_stream;
    for (const auto& line : multiline_buffer_) {
        code_stream << line << "\n";
    }
    
    std::string code = code_stream.str();
    ClearMultiLineBuffer();
    
    // Execute the complete multi-line code
    auto result = python_engine_->ExecutePython(code);
    if (result.result.has_value()) {
        auto& res = result.result.value();
        if (std::holds_alternative<std::string>(res)) {
            last_output_ = std::get<std::string>(res);
            return FormatOutput(last_output_);
        }
    }
    
    if (result.error.has_value()) {
        return FormatOutput("Error: Multi-line execution failed", true);
    }
    
    return ""; // Successful execution with no output
}

void PythonREPL::ClearMultiLineBuffer() {
    multiline_buffer_.clear();
    indent_level_ = 0;
}

bool PythonREPL::NeedsMoreInput(const std::string& input) {
    // If we're in a multi-line context and the input isn't empty
    return indent_level_ > 0 || (!input.empty() && input.back() == ':');
}

int PythonREPL::CountLeadingSpaces(const std::string& line) {
    int count = 0;
    for (char c : line) {
        if (c == ' ') count++;
        else if (c == '\t') count += 4; // Treat tab as 4 spaces
        else break;
    }
    return count;
}

std::string PythonREPL::FormatOutput(const std::string& output, bool is_error) {
    if (output.empty()) {
        return "";
    }
    
    if (is_error) {
        return "ERROR: " + output;
    }
    
    return output;
}

std::string PythonREPL::GeneratePrompt() const {
    if (indent_level_ > 0 || !multiline_buffer_.empty()) {
        return "... ";
    }
    return ">>> ";
}

void PythonREPL::ResetREPLState() {
    ClearMultiLineBuffer();
    command_history_.clear();
    last_output_.clear();
}

std::string PythonREPL::GetVariableList() {
    auto result = python_engine_->ExecutePython("help_vars()");
    if (result.result.has_value() && std::holds_alternative<std::string>(result.result.value())) {
        return std::get<std::string>(result.result.value());
    }
    return "No variables defined";
}

std::string PythonREPL::GetModuleList() {
    auto result = python_engine_->ExecutePython("help_modules()");
    if (result.result.has_value() && std::holds_alternative<std::string>(result.result.value())) {
        return std::get<std::string>(result.result.value());
    }
    return "No modules imported";
}

std::string PythonREPL::DescribeVariable(const std::string& var_name) {
    std::string code = "print(f\"" + var_name + ": {type(" + var_name + ").__name__} = {repr(" + var_name + ")}\")";
    auto result = python_engine_->ExecutePython(code);
    if (result.result.has_value() && std::holds_alternative<std::string>(result.result.value())) {
        return std::get<std::string>(result.result.value());
    }
    return "Variable '" + var_name + "' not found";
}

#endif // ENABLE_PYTHON_FFI
