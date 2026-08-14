// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#pragma once

#include <Python.h>

#include <map>
#include <memory>
#include <optional>
#include <string>

#include "dynamic_calc_types.h"

/**
 * @brief Python FFI Engine for integrating Python functionality
 */
class PythonEngine {
 public:
  PythonEngine();
  ~PythonEngine();

  bool Initialize();
  void Finalize();
  bool IsInitialized() const { return initialized_; }

  EngineResult ExecutePython(const std::string& code);
  EngineResult EvaluatePython(const std::string& expression);

  bool SetVariable(const std::string& name, double value);
  bool SetVariable(const std::string& name, const AXIOM::Vector& values);
  bool SetVariable(const std::string& name, const AXIOM::Matrix& matrix);

  std::optional<double> GetDouble(const std::string& name);
  std::optional<AXIOM::Vector> GetVector(const std::string& name);
  std::optional<AXIOM::Matrix> GetMatrix(const std::string& name);

  EngineResult CreateNumpyArray(const AXIOM::Vector& data);
  EngineResult NumpyOperation(const std::string& operation);

  EngineResult ScipyFunction(const std::string& func_name,
                             const AXIOM::Vector& args);
  EngineResult ScipyOptimize(const std::string& objective,
                             const AXIOM::Vector& initial_guess);

  EngineResult MatplotlibPlot(const std::string& expression, double x_min,
                              double x_max, int points = 1000);
  EngineResult MatplotlibShow();
  EngineResult MatplotlibSaveFig(const std::string& filename);

  EngineResult ImportModule(const std::string& module_name);
  EngineResult ListAvailableModules();
  EngineResult GetPythonVersion();

  std::string GetLastError() const { return last_error_; }
  void ClearError() { last_error_.clear(); }

 private:
  bool initialized_ = false;
  std::string last_error_{};
  std::map<std::string, PyObject*> cached_modules_{};

  PyObject* VectorToPyList(const AXIOM::Vector& vec);
  PyObject* MatrixToPyList(const AXIOM::Matrix& matrix);
  AXIOM::Vector PyListToVector(PyObject* obj);
  AXIOM::Matrix PyListToMatrix(PyObject* obj);

  void SetErrorFromPython();
  bool CheckPythonError();
  PyObject* GetOrImportModule(const std::string& module_name);
  std::string PyObjectToString(PyObject* obj);
  PyObject* StringToPyObject(const std::string& str);
};

enum class PythonMode { Interactive, NumPy, SciPy, Matplotlib, Pandas, SymPy };

struct PythonConfig {
  PythonMode mode = PythonMode::Interactive;
  bool auto_import_numpy = true;
  bool auto_import_scipy = true;
  bool auto_import_matplotlib = true;
  bool show_plots_inline = true;
  std::string plot_backend = "TkAgg";
  int precision = 6;
  bool scientific_notation = false;
};
