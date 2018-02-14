#include "spectral/Spectrum.h"
#include "spectral/SpectrumDescriptor.h"

#include "pypearray.h"
#include <pybind11/numpy.h>
#include <pybind11/operators.h>

using namespace PR;
namespace PRPY {
void setup_spectral(py::module& m)
{
	py::class_<SpectrumDescriptor, std::shared_ptr<SpectrumDescriptor>>(m, "SpectrumDescriptor")
		.def(py::init<uint32, float, float>())
		.def_property_readonly("samples", &SpectrumDescriptor::samples)
		.def_property_readonly("isTriplet", &SpectrumDescriptor::isTriplet)
		.def_property_readonly("isStandardSpectral", &SpectrumDescriptor::isStandardSpectral)
		.def_static("createTriplet", &SpectrumDescriptor::createTriplet)
		.def_static("createStandardSpectral", &SpectrumDescriptor::createStandardSpectral);

	py::class_<Spectrum>(m, "Spectrum", py::buffer_protocol())
					.def(py::init<const std::shared_ptr<SpectrumDescriptor>&>())
					.def_buffer([](Spectrum& s) -> py::buffer_info { // Allow buffer use
						return py::buffer_info(
							s.ptr(),
							sizeof(float),
							py::format_descriptor<float>::format(),
							1,
							std::vector<size_t>({ s.samples() }),
							std::vector<size_t>({ sizeof(float) }));
					})
					.def("fill", (void (Spectrum::*)(float)) & Spectrum::fill)
					.def("fill", (void (Spectrum::*)(uint32, uint32, float)) & Spectrum::fill)
					.def("clear", &Spectrum::clear)
					.def_property_readonly("max", &Spectrum::max)
					.def_property_readonly("min", &Spectrum::min)
					.def_property_readonly("avg", &Spectrum::avg)
					.def("normalize", &Spectrum::normalize)
					.def("normalized", &Spectrum::normalized)
					.def("clamp", &Spectrum::clamp)
					.def("clamped", &Spectrum::clamped)
					.def("lerp", py::overload_cast<const Spectrum&, float>(&Spectrum::lerp))
					.def_static("fromLerp", py::overload_cast<const Spectrum&, const Spectrum&, float>(&Spectrum::lerp))
					.def("sqrt", &Spectrum::sqrt)
					.def("sqrted", &Spectrum::sqrted)
					.def("hasNaN", &Spectrum::hasNaN)
					.def("hasInf", &Spectrum::hasInf)
					.def("hasNegative", &Spectrum::hasNegative)
					.def("isOnlyZero", &Spectrum::isOnlyZero)
					.def("weightPhotometric", &Spectrum::weightPhotometric)
					.def_property_readonly("luminousFlux", &Spectrum::luminousFlux)
					.def_static("black", &Spectrum::black)
					.def_static("white", &Spectrum::white)
					.def_static("gray", &Spectrum::gray)
					.def_static("blackbody", &Spectrum::blackbody)
					.def("__getitem__", &Spectrum::value)
					.def("__setitem__", &Spectrum::setValue)
					.def(py::self + py::self)
					.def(py::self - py::self)
					.def(py::self * py::self)
					.def(py::self * float())
					.def(float() * py::self)
					.def(py::self / py::self)
					.def(py::self / float())
					.def(float() / py::self)
					.def(py::self += py::self)
					.def(py::self -= py::self)
					.def(py::self *= py::self)
					.def(py::self *= float())
					.def(py::self /= py::self)
					.def(py::self /= float())
					.def(py::self == py::self)
					.def(py::self != py::self);
}
}