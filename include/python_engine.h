#pragma once

#include <Python.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <map>
#include "dynamic_calc_types.h"

/**
 * @brief Python FFI Engine for integrating Python functionality
 * 
 * This class provides seamless integration between C++ and Python,
 * enabling access to the entire Python ecosystem including NumPy,
 * SciPy, Matplotlib, and more.
 */
class PythonEngine {
public:
    PythonEngine();
    ~PythonEngine();

    // Core Python operations
    bool Initialize();
    void Finalize();
    bool IsInitialized() const { return initialized_; }

    // Code execution
    EngineResult ExecutePython(const std::string& code);
    EngineResult EvaluatePython(const std::string& expression);
    
    // Variable management
    bool SetVariable(const std::string& name, double value);
    bool SetVariable(const std::string& name, const std::vector<double>& values);
    bool SetVariable(const std::string& name, const Matrix& matrix);
    std::optional<double> GetDouble(const std::string& name);
    std::optional<std::vector<double>> GetVector(const std::string& name);
    std::optional<Matrix> GetMatrix(const std::string& name);

    // NumPy integration
    EngineResult CreateNumpyArray(const std::vector<double>& data);
    EngineResult NumpyOperation(const std::string& operation);
    
    // SciPy integration  
    EngineResult ScipyFunction(const std::string& func_name, const std::vector<double>& args);
    EngineResult ScipyOptimize(const std::string& objective, const std::vector<double>& initial_guess);
    
    // Matplotlib integration
    EngineResult MatplotlibPlot(const std::string& expression, double x_min, double x_max, int points = 1000);
    EngineResult MatplotlibShow();
    EngineResult MatplotlibSaveFig(const std::string& filename);

    // Advanced features
    EngineResult ImportModule(const std::string& module_name);
    EngineResult ListAvailableModules();
    EngineResult GetPythonVersion();
    
    // Error handling
    std::string GetLastError() const { return last_error_; }
    void ClearError() { last_error_.clear(); }

private:
    bool initialized_ = false;
    std::string last_error_{};
    std::map<std::string, PyObject*> cached_modules_{};

    // Python object helpers
    PyObject* VectorToPyList(const std::vector<double>& vec);
    PyObject* MatrixToPyList(const Matrix& matrix);
    std::vector<double> PyListToVector(PyObject* obj);
    Matrix PyListToMatrix(PyObject* obj);
    
    // Error handling helpers
    void SetErrorFromPython();
    bool CheckPythonError();
    
    // Module management
    PyObject* GetOrImportModule(const std::string& module_name);
    
    // Utility functions
    std::string PyObjectToString(PyObject* obj);
    PyObject* StringToPyObject(const std::string& str);
};

/**
 * @brief Python-specific calculation modes
 */
enum class PythonMode {
    Interactive,    // Python REPL mode
    NumPy,         // NumPy array operations
    SciPy,         // Scientific computing
    Matplotlib,    // Advanced plotting
    Pandas,        // Data analysis
    SymPy          // Symbolic mathematics
};

/**
 * @brief Configuration for Python operations
 */
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