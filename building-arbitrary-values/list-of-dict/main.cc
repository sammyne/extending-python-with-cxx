#include <iostream>
#include <memory>
#include <string>

#define PY_SSIZE_T_CLEAN 1

#include <Python.h>

using namespace std;

template <typename T>
using Result = pair<T, int>;

bool appendToSysPath(const char* path);
void freePyObject(PyObject* v);
void mustExitIfAnyPyErr(int status);

using PyPtr = unique_ptr<PyObject, decltype(&freePyObject)>;

Result<PyPtr> buildArgs();
Result<PyPtr> loadFunc(const char* module, const char* func_name);
Result<PyPtr> newErr(int err);
Result<PyPtr> newOk(PyPtr);
PyPtr newPyPtr(PyObject* v);

int main() {
  const auto kModuleName = "main";
  const auto kFuncName = "hello_world";

  auto [fn, err] = loadFunc(kModuleName, kFuncName);
  if (0 != err) {
    cout << "fail to load func: " << err << endl;
    mustExitIfAnyPyErr(1);
    return 1;
  }

  auto [args, err1] = buildArgs();
  if (0 != err1) {
    cout << "fail to build args: " << err1 << endl;
    return 2;
  }
  mustExitIfAnyPyErr(1);

  PyObject_CallObject(fn.get(), args.get());
  mustExitIfAnyPyErr(1);

  return 0;
}

bool appendToSysPath(const char* path) {
  string cmd = "import sys; sys.path.append('";
  cmd += path;
  cmd += "')";

  return 0 == PyRun_SimpleString(cmd.c_str());
}

void freePyObject(PyObject* v) { Py_CLEAR(v); }

Result<PyPtr> loadFunc(const char* module_name, const char* func_name) {
  Py_Initialize();
  if (!Py_IsInitialized()) {
    return newErr(1);
  }

  if (!appendToSysPath("./py")) {
    return newErr(2);
  }

  auto m = PyImport_ImportModule(module_name);
  if (nullptr == m) {
    return newErr(3);
  }

  auto fn = PyObject_GetAttrString(m, func_name);
  if ((nullptr == fn) || !PyCallable_Check(fn)) {
    return newErr(4);
  }

  return newOk(newPyPtr(fn));
}

void mustExitIfAnyPyErr(int status) {
  if (!PyErr_Occurred()) {
    return;
  }

  PyErr_Print();
  exit(status);
}

Result<PyPtr> buildArgs() {
  const int kListLen = 2;

  auto arr = newPyPtr(PyList_New(kListLen));
  for (auto i = 0; i < kListLen; ++i) {
    string opaque = "opaque " + to_string(i);
    auto v = Py_BuildValue("{s:i,s:y#}", "value", i, "opaque", opaque.c_str(), opaque.size());
    if (auto err = PyList_SetItem(arr.get(), i, v); 0 != err) {
      return newErr(err);
    }
  }

  auto out = newPyPtr(PyTuple_New(1));
  if (auto err = PyTuple_SetItem(out.get(), 0, arr.get()); 0 != err) {
    return newErr(2);
  }
  arr.release();

  return newOk(std::move(out));
}

Result<PyPtr> newErr(int err) {
  return std::make_pair<PyPtr, int>(newPyPtr(nullptr), std::move(err));
}

Result<PyPtr> newOk(PyPtr v) { return std::make_pair<PyPtr, int>(std::move(v), 0); }

PyPtr newPyPtr(PyObject* v) { return PyPtr(v, freePyObject); }
