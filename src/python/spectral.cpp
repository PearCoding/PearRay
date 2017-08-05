#include "spectral/IntensityConverter.h"
#include "spectral/RGBConverter.h"
#include "spectral/Spectrum.h"
#include "spectral/XYZConverter.h"

#include "pypearray.h"
#include <pybind11/numpy.h>
#include <pybind11/operators.h>

using namespace PR;
namespace PRPY {
class XYZConverterWrap : public XYZConverter {
public:
	static Eigen::Vector2f convert(const Spectrum& spec)
	{
		float x, y;
		XYZConverter::convert(spec, x, y);
		return Eigen::Vector2f(x, y);
	}
	static Eigen::Vector3f convertXYZ(const Spectrum& spec)
	{
		float x, y, z;
		XYZConverter::convertXYZ(spec, x, y, z);
		return Eigen::Vector3f(x, y, z);
	}
	static Eigen::Vector2f toNorm(float X, float Y, float Z)
	{
		float x, y;
		XYZConverter::toNorm(X, Y, Z, x, y);
		return Eigen::Vector2f(x, y);
	}
};

class RGBConverterWrap : public RGBConverter {
public:
	static Eigen::Vector3f convert(const Spectrum& spec)
	{
		float x, y, z;
		RGBConverter::convert(spec, x, y, z);
		return Eigen::Vector3f(x, y, z);
	}

	static Eigen::Vector3f toXYZ(float X, float Y, float Z)
	{
		float x, y, z;
		RGBConverter::toXYZ(X, Y, Z, x, y, z);
		return Eigen::Vector3f(x, y, z);
	}

	static Eigen::Vector3f fromXYZ(float X, float Y, float Z)
	{
		float x, y, z;
		RGBConverter::fromXYZ(X, Y, Z, x, y, z);
		return Eigen::Vector3f(x, y, z);
	}

	static Eigen::Vector3f gamma(float X, float Y, float Z)
	{
		float x = X, y = Y, z = Z;
		RGBConverter::gamma(x, y, z);
		return Eigen::Vector3f(x, y, z);
	}
};

void setup_spectral(py::module& m)
{
	auto spec = py::class_<Spectrum>(m, "Spectrum", py::buffer_protocol())
					.def(py::init<float>())
					.def("__init__", [](Spectrum& s, py::array_t<float, py::array::c_style | py::array::forcecast> b) { // Allow buffer initializing
						py::buffer_info info = b.request();
						if (info.format != py::format_descriptor<float>::format())
							throw std::runtime_error("Incompatible format: expected a float array!");

						if (info.ndim != 1)
							throw std::runtime_error("Incompatible buffer dimension. Expected 1d");

						if (info.itemsize != sizeof(float))
							throw std::runtime_error("Incompatible format: Expected float item size");

						if (info.shape[0] != Spectrum::SAMPLING_COUNT)
							throw std::runtime_error("Incompatible shape: Expected correct sampling count");

						new (&s) Spectrum((float*)info.ptr);
					})
					.def_buffer([](Spectrum& s) -> py::buffer_info { // Allow buffer use
						return py::buffer_info(
							s.ptr(),
							sizeof(float),
							py::format_descriptor<float>::format(),
							1,
							std::vector<size_t>({ Spectrum::SAMPLING_COUNT }),
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
					.def("lerp", (Spectrum & (Spectrum::*)(const Spectrum&, float)) & Spectrum::lerp)
					.def_static("fromLerp", (Spectrum(*)(const Spectrum&, const Spectrum&, float)) & Spectrum::lerp)
					.def("sqrt", &Spectrum::sqrt)
					.def("sqrted", &Spectrum::sqrted)
					.def("hasNaN", &Spectrum::hasNaN)
					.def("hasInf", &Spectrum::hasInf)
					.def("hasNegative", &Spectrum::hasNegative)
					.def("isOnlyZero", &Spectrum::isOnlyZero)
					.def("weightPhotometric", &Spectrum::weightPhotometric)
					.def_property_readonly("luminousFlux", &Spectrum::luminousFlux)
					.def_static("fromBlackbody", &Spectrum::fromBlackbody)
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
	spec.attr("WAVELENGTH_START")	 = py::int_(Spectrum::WAVELENGTH_START);
	spec.attr("WAVELENGTH_END")		  = py::int_(Spectrum::WAVELENGTH_END);
	spec.attr("WAVELENGTH_AREA_SIZE") = py::int_(Spectrum::WAVELENGTH_AREA_SIZE);
	spec.attr("WAVELENGTH_STEP")	  = py::int_(Spectrum::WAVELENGTH_STEP);
	spec.attr("SAMPLING_COUNT")		  = py::int_(Spectrum::SAMPLING_COUNT);
	spec.attr("ILL_SCALE")			  = py::float_(Spectrum::ILL_SCALE);

	py::class_<IntensityConverter>(m, "IntensityConverter")
		.def_static("convert", &IntensityConverter::convert);

	py::class_<XYZConverterWrap>(m, "XYZConverter")
		.def_static("convert", &XYZConverterWrap::convert)
		.def_static("convertXYZ", &XYZConverterWrap::convertXYZ)
		.def_static("luminance", &XYZConverter::luminance)
		.def_static("toNorm", &XYZConverterWrap::toNorm)
		.def_static("toSpec", &XYZConverter::toSpec);

	py::class_<RGBConverterWrap>(m, "RGBConverter")
		.def_static("convert", &RGBConverterWrap::convert)
		.def_static("toXYZ", &RGBConverterWrap::toXYZ)
		.def_static("fromXYZ", &RGBConverterWrap::fromXYZ)
		.def_static("luminance", &RGBConverter::luminance)
		.def_static("gamma", &RGBConverterWrap::gamma)
		.def_static("toSpec", &RGBConverter::toSpec);
}
}