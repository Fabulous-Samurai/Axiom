// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once

#include "IParser.h"
#include "python_engine.h"

namespace AXIOM {

class PythonParser : public IParser {
 public:
  PythonParser() : python_engine_(nullptr), mode_(PythonMode::Interactive) {}
  explicit PythonParser(PythonEngine* engine,
                        PythonMode mode = PythonMode::Interactive);
  ~PythonParser() = default;

  EngineResult ParseAndExecute(std::string_view input) noexcept override;

 private:
  PythonEngine* python_engine_;
  PythonMode mode_;

  // Mode-specific handlers
  std::string HandleInteractiveMode(std::string_view input);
  std::string HandleNumPyMode(std::string_view input);
  std::string HandleSciPyMode(std::string_view input);
  std::string HandleMatplotlibMode(std::string_view input);
  std::string HandlePandasMode(std::string_view input);
  std::string HandleSymPyMode(std::string_view input);
};

}  // namespace AXIOM
