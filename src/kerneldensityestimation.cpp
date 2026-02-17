#include "kerneldensityestimation.hpp"
#include "typedefs.hpp"
#include "../hyperparameters.hpp"
#include "kernels.hpp"


PYBIND11_MODULE(kerneldensityestimation, m) {

	py::class_<KernelDensityEstimation<KernelTypeKDE>>(m, "KernelDensityEstimation")
	
		.def(py::init([](const np_array<double> & values, double bandwidth, double a, double b){
			return KernelDensityEstimation<KernelTypeKDE>(values.data(), values.shape(0), bandwidth, a, b);
		}), py::arg("values"), py::arg("bandwidth")=1, py::arg("a")=-infinity, py::arg("b")=infinity)

		.def(py::init([](const np_array<double>& values, const np_array<double>& bins, double bandwidth, double a, double b){
			return KernelDensityEstimation<KernelTypeKDE>(values.data(), values.shape(0), bins.data(), bins.shape(0), bandwidth, a, b);
		}), py::arg("values"), py::arg("bins"), py::arg("bandwidth")=1, py::arg("a")=-infinity, py::arg("b")=infinity)
		
		.def("__call__", py::vectorize(&KernelDensityEstimation<KernelTypeKDE>::operator()), py::arg("x"))

		.def("evalUnnormalized", py::vectorize(&KernelDensityEstimation<KernelTypeKDE>::evalUnnormalized), py::arg("x"))
		
		.def("binValues", [](KernelDensityEstimation<KernelTypeKDE> & self, const np_array<double> & bin_edges) {
			self.binValues(std::vector<double>(bin_edges.data(), bin_edges.data() + bin_edges.shape(0)));
		}, py::arg("bin_edges"))
	
	;

}