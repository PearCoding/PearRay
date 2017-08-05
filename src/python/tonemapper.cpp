#include "spectral/Spectrum.h"
#include "spectral/ToneMapper.h"

#include "SpectralFile.h"
#include "renderer/OutputChannel.h"

#include "pypearray.h"
#include <pybind11/numpy.h>

using namespace PR;
namespace PRPY {
class ToneMapperWrap : public ToneMapper {
public:
	ToneMapperWrap(uint32 w, uint32 h)
		: ToneMapper(w, h, nullptr)
	{
	}

	typedef py::array_t<float, py::array::c_style | py::array::forcecast> Array;

	Array map_py(Array specIn) const
	{
		// specIn
		py::buffer_info info1 = specIn.request();
		if (info1.format != py::format_descriptor<float>::format())
			throw std::runtime_error("Incompatible format: expected a float array!");

		if (info1.ndim != 3)
			throw std::runtime_error("Incompatible buffer dimension. Expected 3d");

		if (info1.itemsize != sizeof(float))
			throw std::runtime_error("Incompatible format: Expected float item size");

		if (info1.shape[0] != height() || info1.shape[1] != width())
			throw std::runtime_error("Incompatible shape: Outermost dimensions do not equal tone mapper");

		if (info1.shape[2] != Spectrum::SAMPLING_COUNT)
			throw std::runtime_error("Incompatible shape: Expected correct sampling count in inner most dimension");

		// rgbOut
		float* mem = new float[width() * height() * 3];
		map((Spectrum*)info1.ptr, mem);

		py::capsule free_when_done(mem, [](void* f) {
			delete[] reinterpret_cast<float*>(f);
		});

		return py::array_t<float>(
			std::vector<size_t>({ height(), width(), 3 }),
			std::vector<size_t>({ width() * 3 * sizeof(float), 3 * sizeof(float), sizeof(float) }),
			mem,
			free_when_done);
	}

	Array mapOnlyMapper_py(Array rgbIn) const
	{
		// specIn
		py::buffer_info info1 = rgbIn.request();
		if (info1.format != py::format_descriptor<float>::format())
			throw std::runtime_error("Incompatible format: expected a float array!");

		if (info1.ndim != 3)
			throw std::runtime_error("Incompatible buffer dimension. Expected 2d");

		if (info1.itemsize != sizeof(float))
			throw std::runtime_error("Incompatible format: Expected float item size");

		if (info1.shape[0] != height() || info1.shape[1] != width())
			throw std::runtime_error("Incompatible shape: Outermost dimensions do not equal tone mapper");

		if (info1.shape[2] != 3)
			throw std::runtime_error("Incompatible shape: Expected RGB in inner most dimension");

		// rgbOut
		float* mem = new float[width() * height() * 3]; // Other way??
		mapOnlyMapper((float*)info1.ptr, mem);

		py::capsule free_when_done(mem, [](void* f) {
			delete[] reinterpret_cast<float*>(f);
		});

		return py::array_t<float>(
			std::vector<size_t>({ height(), width(), 3 }),
			std::vector<size_t>({ width() * 3 * sizeof(float), 3 * sizeof(float), sizeof(float) }),
			mem,
			free_when_done);
	}
};

void setup_tonemapper(py::module& m)
{
	py::class_<ToneMapper, ToneMapperWrap>(m, "ToneMapper")
		.def(py::init<uint32, uint32>())
		.def("map", &ToneMapperWrap::map_py)
		.def("mapOnlyMapper", &ToneMapperWrap::mapOnlyMapper_py)
		.def_property("colorMode", &ToneMapper::colorMode, &ToneMapper::setColorMode)
		.def_property("gammaMode", &ToneMapper::gammaMode, &ToneMapper::setGammaMode)
		.def_property("mapperMode", &ToneMapper::mapperMode, &ToneMapper::setMapperMode)
		.def_property_readonly("width", &ToneMapper::width)
		.def_property_readonly("height", &ToneMapper::height);

	py::enum_<ToneColorMode>(m, "ToneColorMode")
		.value("SRGB", TCM_SRGB)
		.value("XYZ", TCM_XYZ)
		.value("XYZ_NORM", TCM_XYZ_NORM)
		.value("LUMINANCE", TCM_LUMINANCE);

	py::enum_<ToneGammaMode>(m, "ToneGammaMode")
		.value("NONE", TGM_None)
		.value("SRGB", TGM_SRGB);

	py::enum_<ToneMapperMode>(m, "ToneMapperMode")
		.value("NONE", TMM_None)
		.value("SIMPLE_REINHARD", TMM_Simple_Reinhard)
		.value("CLAMP", TMM_Clamp)
		.value("ABS", TMM_Abs)
		.value("POSITIVE", TMM_Positive)
		.value("NEGATIVE", TMM_Negative)
		.value("SPHERICAL", TMM_Spherical)
		.value("NORMALIZED", TMM_Normalized);
}
}