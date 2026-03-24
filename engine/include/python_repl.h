// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#ifdef ENABLE_PYTHON_FFI

#include "python_engine.h"
#include <string>
#include <vector>

class PythonREPL {
public:
    explicit PythonREPL(PythonEngine* engine);
    ~PythonREPL() = default;

    // Interactive REPL functionality
    void StartInteractiveSession();
    void StopInteractiveSession();
    bool IsSessionActive() const { return session_active_; }
    
    // Execute Python code with REPL-like features
    std::string ExecuteInteractive(const std::string& input);
    
    // Handle multi-line input (for functions, classes, etc.)
    bool IsMultiLineInput(const std::string& input);
    void AddToMultiLineBuffer(const std::string& input);
    std::string ExecuteMultiLineBuffer();
    void ClearMultiLineBuffer();
    
    // REPL state management
    void ResetREPLState();
    std::vector<std::string> GetCommandHistory() const { return command_history_; }
    std::string GetLastOutput() const { return last_output_; }
    
    // Python environment introspection
    std::string GetVariableList();
    std::string GetModuleList();
    std::string DescribeVariable(const std::string& var_name);

private:
    PythonEngine* python_engine_;
    bool session_active_;
    
    // Multi-line input handling
    std::vector<std::string> multiline_buffer_;
    int indent_level_;
    
    // REPL history and state
    std::vector<std::string> command_history_;
    std::string last_output_;
    
    // Helper methods
    bool NeedsMoreInput(const std::string& input);
    int CountLeadingSpaces(const std::string& line);
    std::string FormatOutput(const std::string& output, bool is_error = false);
    std::string GeneratePrompt() const;
};

#endif // ENABLE_PYTHON_FFI
