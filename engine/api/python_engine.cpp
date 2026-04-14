// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#include "../include/python_engine.h"

#include <algorithm>
#include <iostream>
#include <vector>

PythonEngine::PythonEngine() : initialized_(false) {}

PythonEngine::~PythonEngine() { Finalize(); }

bool PythonEngine::Initialize() {
  if (initialized_) return true;

  Py_Initialize();
  if (!Py_IsInitialized()) {
    last_error_ = "Failed to initialize Python interpreter";
    return false;
  }

  // Import common modules
  GetOrImportModule("numpy");
  GetOrImportModule("scipy");

  initialized_ = true;
  return true;
}

void PythonEngine::Finalize() {
  if (!initialized_) return;

  for (auto const& [name, module] : cached_modules_) {
    Py_XDECREF(module);
  }
  cached_modules_.clear();

  Py_Finalize();
  initialized_ = false;
}

EngineResult PythonEngine::ExecutePython(const std::string& code) {
  if (!IsInitialized()) return CreateErrorResult(CalcErr::OperationNotFound);

  int result = PyRun_SimpleString(code.c_str());
  if (result != 0) {
    last_error_ = "Python execution error";
    return CreateErrorResult(CalcErr::OperationNotFound);
  }

  return CreateSuccessResult(0.0);
}

EngineResult PythonEngine::EvaluatePython(const std::string& expression) {
  if (!IsInitialized()) return CreateErrorResult(CalcErr::OperationNotFound);

  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* global_dict = PyModule_GetDict(main_module);

  PyObject* result_obj =
      PyRun_String(expression.c_str(), Py_eval_input, global_dict, global_dict);

  if (!result_obj) {
    SetErrorFromPython();
    return CreateErrorResult(CalcErr::OperationNotFound);
  }

  EngineResult res;
  if (PyFloat_Check(result_obj)) {
    res = CreateSuccessResult(PyFloat_AsDouble(result_obj));
  } else if (PyLong_Check(result_obj)) {
    res = CreateSuccessResult(static_cast<double>(PyLong_AsLong(result_obj)));
  } else if (PyList_Check(result_obj)) {
    res = CreateSuccessResult(PyListToVector(result_obj));
  } else {
    res = CreateErrorResult(CalcErr::OperationNotFound);
  }

  Py_XDECREF(result_obj);
  return res;
}

bool PythonEngine::SetVariable(const std::string& name, double value) {
  if (!IsInitialized()) return false;

  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* global_dict = PyModule_GetDict(main_module);

  PyObject* py_val = PyFloat_FromDouble(value);
  int result = PyDict_SetItemString(global_dict, name.c_str(), py_val);
  Py_XDECREF(py_val);

  return result == 0;
}

bool PythonEngine::SetVariable(const std::string& name,
                               const AXIOM::Vector& values) {
  if (!IsInitialized()) return false;

  PyObject* py_list = VectorToPyList(values);
  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* global_dict = PyModule_GetDict(main_module);

  int result = PyDict_SetItemString(global_dict, name.c_str(), py_list);
  Py_XDECREF(py_list);

  return result == 0;
}

bool PythonEngine::SetVariable(const std::string& name,
                               const AXIOM::Matrix& matrix) {
  if (!IsInitialized()) return false;

  PyObject* py_list = MatrixToPyList(matrix);
  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* global_dict = PyModule_GetDict(main_module);

  int result = PyDict_SetItemString(global_dict, name.c_str(), py_list);
  Py_XDECREF(py_list);

  return result == 0;
}

std::optional<double> PythonEngine::GetDouble(const std::string& name) {
  if (!IsInitialized()) return std::nullopt;

  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* global_dict = PyModule_GetDict(main_module);
  PyObject* py_val = PyDict_GetItemString(global_dict, name.c_str());

  if (py_val && PyFloat_Check(py_val)) {
    return PyFloat_AsDouble(py_val);
  }
  return std::nullopt;
}

std::optional<AXIOM::Vector> PythonEngine::GetVector(const std::string& name) {
  if (!IsInitialized()) return std::nullopt;

  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* global_dict = PyModule_GetDict(main_module);
  PyObject* py_val = PyDict_GetItemString(global_dict, name.c_str());

  if (py_val && PyList_Check(py_val)) {
    return PyListToVector(py_val);
  }
  return std::nullopt;
}

std::optional<AXIOM::Matrix> PythonEngine::GetMatrix(const std::string& name) {
  if (!IsInitialized()) return std::nullopt;

  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* global_dict = PyModule_GetDict(main_module);
  PyObject* py_val = PyDict_GetItemString(global_dict, name.c_str());

  if (py_val && PyList_Check(py_val)) {
    return PyListToMatrix(py_val);
  }
  return std::nullopt;
}

EngineResult PythonEngine::CreateNumpyArray(const AXIOM::Vector& data) {
  if (!IsInitialized()) return CreateErrorResult(CalcErr::OperationNotFound);

  PyObject* py_list = VectorToPyList(data);
  SetVariable("__tmp_list", data);
  ExecutePython("import numpy as np\n__tmp_arr = np.array(__tmp_list)");

  return CreateSuccessResult(0.0);
}

EngineResult PythonEngine::NumpyOperation(const std::string& operation) {
  return ExecutePython("import numpy as np\n" + operation);
}

EngineResult PythonEngine::ScipyFunction(const std::string& func_name,
                                         const AXIOM::Vector& args) {
  SetVariable("__tmp_args", args);
  return ExecutePython("import scipy as sp\n__tmp_res = sp." + func_name +
                       "(*__tmp_args)");
}

EngineResult PythonEngine::ScipyOptimize(const std::string& objective,
                                         const AXIOM::Vector& initial_guess) {
  SetVariable("__tmp_x0", initial_guess);
  return ExecutePython(
      "from scipy.optimize import minimize\n__tmp_res = minimize(" + objective +
      ", __tmp_x0)");
}

EngineResult PythonEngine::MatplotlibPlot(const std::string& expression,
                                          double x_min, double x_max,
                                          int points) {
  std::string code =
      "import matplotlib.pyplot as plt\n"
      "import numpy as np\n"
      "x = np.linspace(" +
      std::to_string(x_min) + "," + std::to_string(x_max) + "," +
      std::to_string(points) +
      ")\n"
      "y = " +
      expression +
      "\n"
      "plt.plot(x, y)\n";
  return ExecutePython(code);
}

EngineResult PythonEngine::MatplotlibShow() {
  return ExecutePython("plt.show()");
}

EngineResult PythonEngine::MatplotlibSaveFig(const std::string& filename) {
  return ExecutePython("plt.savefig('" + filename + "')");
}

EngineResult PythonEngine::ImportModule(const std::string& module_name) {
  if (GetOrImportModule(module_name)) return CreateSuccessResult(0.0);
  return CreateErrorResult(CalcErr::OperationNotFound);
}

EngineResult PythonEngine::ListAvailableModules() {
  return CreateSuccessResult("numpy, scipy, matplotlib, pandas, sympy");
}

EngineResult PythonEngine::GetPythonVersion() {
  return CreateSuccessResult(Py_GetVersion());
}

PyObject* PythonEngine::VectorToPyList(const AXIOM::Vector& vec) {
  PyObject* list = PyList_New(static_cast<Py_ssize_t>(vec.size()));
  for (size_t i = 0; i < vec.size(); ++i) {
    PyList_SetItem(list, static_cast<Py_ssize_t>(i),
                   PyFloat_FromDouble(vec[i]));
  }
  return list;
}

PyObject* PythonEngine::MatrixToPyList(const AXIOM::Matrix& matrix) {
  PyObject* list = PyList_New(static_cast<Py_ssize_t>(matrix.size()));
  for (size_t i = 0; i < matrix.size(); ++i) {
    PyList_SetItem(list, static_cast<Py_ssize_t>(i), VectorToPyList(matrix[i]));
  }
  return list;
}

AXIOM::Vector PythonEngine::PyListToVector(PyObject* obj) {
  AXIOM::Vector vec;
  if (!PyList_Check(obj)) return vec;

  Py_ssize_t size = PyList_Size(obj);
  for (Py_ssize_t i = 0; i < size; ++i) {
    PyObject* item = PyList_GetItem(obj, i);
    if (PyFloat_Check(item))
      vec.push_back(PyFloat_AsDouble(item));
    else if (PyLong_Check(item))
      vec.push_back(static_cast<double>(PyLong_AsLong(item)));
  }
  return vec;
}

AXIOM::Matrix PythonEngine::PyListToMatrix(PyObject* obj) {
  AXIOM::Matrix mat;
  if (!PyList_Check(obj)) return mat;

  Py_ssize_t size = PyList_Size(obj);
  for (Py_ssize_t i = 0; i < size; ++i) {
    mat.push_back(PyListToVector(PyList_GetItem(obj, i)));
  }
  return mat;
}

void PythonEngine::SetErrorFromPython() {
  if (PyErr_Occurred()) {
    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    last_error_ = PyObjectToString(value);
    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
  }
}

bool PythonEngine::CheckPythonError() { return PyErr_Occurred() != nullptr; }

PyObject* PythonEngine::GetOrImportModule(const std::string& module_name) {
  if (cached_modules_.count(module_name)) return cached_modules_[module_name];

  PyObject* module = PyImport_ImportModule(module_name.c_str());
  if (module) cached_modules_[module_name] = module;
  return module;
}

std::string PythonEngine::PyObjectToString(PyObject* obj) {
  if (!obj) return "";
  PyObject* str_obj = PyObject_Str(obj);
  if (!str_obj) return "";
  const char* str = PyUnicode_AsUTF8(str_obj);
  std::string res = str ? str : "";
  Py_XDECREF(str_obj);
  return res;
}

PyObject* PythonEngine::StringToPyObject(const std::string& str) {
  return PyUnicode_FromString(str.c_str());
}
