#pragma once

#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
namespace py = pybind11;

// Define a type alias for numpy arrays
template<typename T>
using np_array = py::array_t<T, py::array::c_style>;