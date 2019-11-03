#include "spectral/Spectrum.h"
#include "spectral/SpectrumDescriptor.h"

#include "pypearray.h"
#include <pybind11/numpy.h>

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
		.def("fill", (void (Spectrum::*)(size_t, size_t, float)) & Spectrum::fill)
		.def("clear", &Spectrum::clear)
		.def_property_readonly("max", &Spectrum::max)
		.def_property_readonly("min", &Spectrum::min)
		.def_property_readonly("avg", &Spectrum::avg)
		.def("normalize", &Spectrum::normalize)
		.def("normalized", &Spectrum::normalized)
		.def("clamp", &Spectrum::clamp)
		.def("clamped", &Spectrum::clamped)
		.def("lerp", [](Spectrum& a, const Spectrum& b, float t) { a.lerp(b, t); })
		.def_static("fromLerp", [](const Spectrum& a, const Spectrum& b, float t) { return Spectrum::lerp(a, b, t); })
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
		.def("__add__", [](const Spectrum& a, const Spectrum& b) {
			return Spectrum(a + b);
		})
		.def("__sub__", [](const Spectrum& a, const Spectrum& b) {
			return Spectrum(a - b);
		})
		.def("__mul__", [](const Spectrum& a, const Spectrum& b) {
			return Spectrum(a * b);
		})
		.def("__mul__", [](const Spectrum& a, float f) {
			return Spectrum(a * f);
		})
		.def("__mul__", [](float f, const Spectrum& b) {
			return Spectrum(b * f);
		})
		.def("__truediv__", [](const Spectrum& a, const Spectrum& b) {
			return Spectrum(a / b);
		})
		.def("__truediv__", [](const Spectrum& a, float f) {
			return Spectrum(a / f);
		})
		.def("__truediv__", [](float f, const Spectrum& b) {
			return Spectrum(f / b);
		})
		.def("__neg__", [](const Spectrum& b) {
			return Spectrum(-b);
		})
		.def("__iadd__", [](Spectrum& a, const Spectrum& b) {
			return a += b;
		})
		.def("__isub__", [](Spectrum& a, const Spectrum& b) {
			return a -= b;
		})
		.def("__imul__", [](Spectrum& a, const Spectrum& b) {
			return a *= b;
		})
		.def("__imul__", [](Spectrum& a, float f) {
			return a *= f;
		})
		.def("__itruediv__", [](Spectrum& a, const Spectrum& b) {
			return a /= b;
		})
		.def("__itruediv__", [](Spectrum& a, float f) {
			return a /= f;
		})
		.def("__eq__", [](const Spectrum& a, const Spectrum& b) {
			return a == b;
		})
		.def("__neq__", [](const Spectrum& a, const Spectrum& b) {
			return a != b;
		});
}
} // namespace PRPY