#include "fittingfunctions.hpp"
#include "typedefs.hpp"


PYBIND11_MODULE(fittingfunctions, m) {
	m.doc() = "Fitting functions module";

	m.def("velocityCorrection", py::vectorize(&velocityCorrection), 
		pybind11::arg("R"), pybind11::arg("a"), pybind11::arg("b"), 
		pybind11::arg("c"), pybind11::arg("m"), pybind11::arg("s"), 
		pybind11::arg("d"), pybind11::arg("h")
	);

	m.def("logSigmaFittingCurve", py::vectorize(&logSigmaFittingCurve), 
		pybind11::arg("M"), pybind11::arg("logM0"), pybind11::arg("logA")
	);

	m.def("scaleFittingCurve", py::vectorize(&scaleFittingCurve), 
		pybind11::arg("R"), pybind11::arg("M"), pybind11::arg("f_0"), 
		pybind11::arg("a"), pybind11::arg("logM0"), pybind11::arg("logA")
	);

	m.def("invdegFittingCurve", py::vectorize(&invdegFittingCurve), 
		pybind11::arg("R"), pybind11::arg("a"), pybind11::arg("b"), 
		pybind11::arg("c"), pybind11::arg("d"), pybind11::arg("m"), 
		pybind11::arg("s")
	);

	m.def("skewFittingCurve", py::vectorize(&skewFittingCurve), 
		pybind11::arg("R"), pybind11::arg("a"), pybind11::arg("b"), 
		pybind11::arg("c")
	);

	py::class_<PVcorrclass>(m, "PVcorrclass")
		.def(py::init<double, double, double, double, double, double, double>(),
			pybind11::arg("a"), pybind11::arg("b"), pybind11::arg("c"), pybind11::arg("m"),
			pybind11::arg("s"), pybind11::arg("d"), pybind11::arg("h")
		)
		.def("__call__", py::vectorize(&PVcorrclass::operator()), 
			pybind11::arg("R")
		)
	;

	py::class_<PUclass>(m, "PUclass")
		.def(py::init<double, double, double, double, double, double, double, double, double, double>(),
			pybind11::arg("f_0"), pybind11::arg("a_scale"), pybind11::arg("logM0_scale"), pybind11::arg("logA_scale"),
			pybind11::arg("a_invdeg"), pybind11::arg("b_invdeg"), pybind11::arg("c_invdeg"), pybind11::arg("d_invdeg"),
			pybind11::arg("m_invdeg"), pybind11::arg("s_invdeg")
		)
		.def("__call__", py::vectorize(&PUclass::operator()), 
			pybind11::arg("u"), pybind11::arg("R"), pybind11::arg("M")
		)
	;

	py::class_<unnormalizedPosteriorClass>(m, "unnormalizedPosteriorClass")
		.def(py::init<const KernelDensityEstimation<KernelTypeKDE> &, const PVcorrclass &, const PUclass &, double>(),
			pybind11::arg("PR"), pybind11::arg("PVcorr"), pybind11::arg("PU"), pybind11::arg("H0")
		)
		.def("__call__", py::vectorize(&unnormalizedPosteriorClass::operator()), 
			pybind11::arg("R"), pybind11::arg("r"), pybind11::arg("v"), pybind11::arg("M")
		)
		.def("geometricComponent", py::vectorize(&unnormalizedPosteriorClass::geometricComponent), 
			pybind11::arg("R"), pybind11::arg("r")
		)
		.def("velocityComponent", py::vectorize(&unnormalizedPosteriorClass::velocityComponent), 
			pybind11::arg("R"), pybind11::arg("r"), pybind11::arg("v"), pybind11::arg("M")
		)
	;
	
}