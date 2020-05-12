#include "spectral/ToneMapper.h"
#include "buffer/ColorBuffer.h"
#include "buffer/FrameBuffer.h"
#include "spectral/SpectralFile.h"
#include "spectral/Spectrum.h"
#include "spectral/SpectrumDescriptor.h"

#include "pypearray.h"
#include <pybind11/numpy.h>

using namespace PR;
namespace PRPY {
PR_NO_SANITIZE_ADDRESS
void setup_tonemapper(py::module& m)
{
	typedef py::array_t<float, py::array::c_style | py::array::forcecast> Array;

	py::enum_<ColorBufferMode>(m, "ColorBufferMode")
		.value("RGB", CBM_RGB)
		.value("RGBA", CBM_RGBA);

	py::class_<ColorBuffer>(m, "ColorBuffer", py::buffer_protocol())
		.def(py::init<uint32, uint32, ColorBufferMode>(),
			 py::arg("width"), py::arg("height"), py::arg("mode") = CBM_RGBA)
		.def_buffer([](ColorBuffer& s) -> py::buffer_info { // Allow buffer use
			return py::buffer_info(
				s.ptr(),
				sizeof(float),
				py::format_descriptor<float>::format(),
				3,
				std::vector<size_t>({ s.height(),
									  s.width(),
									  s.channels() }),
				std::vector<size_t>({ s.heightBytePitch(),
									  s.widthBytePitch(),
									  s.channelBytePitch() }));
		})
		.def("map", [](ColorBuffer& c,
					   const ToneMapper& mapper,
					   const std::shared_ptr<SpectrumDescriptor>& desc,
					   Array specIn) {
			py::buffer_info info1 = specIn.request();
			if (info1.format != py::format_descriptor<float>::format())
				throw std::runtime_error("Incompatible format: expected a float array!");

			if (info1.ndim != 3)
				throw std::runtime_error("Incompatible buffer dimension. Expected 3d");

			if (info1.itemsize != sizeof(float))
				throw std::runtime_error("Incompatible format: Expected float item size");

			if (info1.shape[2] != (ssize_t)desc->samples())
				throw std::runtime_error("Incompatible descriptor: Expected sample size is not equal channel count");

			if ((size_t)info1.shape[0] != c.height()
				|| (size_t)info1.shape[1] != c.width())
				throw std::runtime_error("Incompatible shape: Outermost dimensions do not equal tone mapper");

			c.map(mapper, desc, (float*)info1.ptr);
		})
		.def("mapOnlyMapper", [](ColorBuffer& c, const ToneMapper& mapper, Array rgbIn) {
			py::buffer_info info1 = rgbIn.request();
			if (info1.format != py::format_descriptor<float>::format())
				throw std::runtime_error("Incompatible format: expected a float array!");

			if (info1.ndim != 3)
				throw std::runtime_error("Incompatible buffer dimension. Expected 3d");

			if (info1.itemsize != sizeof(float))
				throw std::runtime_error("Incompatible format: Expected float item size");

			if ((size_t)info1.shape[0] != c.height()
				|| (size_t)info1.shape[1] != c.width())
				throw std::runtime_error("Incompatible shape: Outermost dimensions do not equal tone mapper");

			if ((size_t)info1.shape[2] != c.channels())
				throw std::runtime_error("Incompatible shape: Expected RGB(A) in inner most dimension");

			c.mapOnlyMapper(mapper, (float*)info1.ptr);
		})
		.def("asLinearWithChannels", [](const ColorBuffer& c) {
			return py::array_t<float>(
				std::vector<size_t>({ c.width() * c.height(), c.channels() }),
				std::vector<size_t>({ c.widthBytePitch(),
									  c.channelBytePitch() }),
				c.ptr());
		})
		.def("flipY", &ColorBuffer::flipY)
		.def_property_readonly("mode", &ColorBuffer::mode)
		.def_property_readonly("width", &ColorBuffer::width)
		.def_property_readonly("height", &ColorBuffer::height);

	py::class_<ToneMapper>(m, "ToneMapper")
		.def(py::init<>())
		.def("map", [](ToneMapper& tm,
					   const std::shared_ptr<SpectrumDescriptor>& desc,
					   Array specIn, size_t elems) {
			// specIn
			py::buffer_info info1 = specIn.request();
			if (info1.format != py::format_descriptor<float>::format())
				throw std::runtime_error("Incompatible format: expected a float array!");

			if (info1.ndim != 3)
				throw std::runtime_error("Incompatible buffer dimension. Expected 3d");

			if (info1.itemsize != sizeof(float))
				throw std::runtime_error("Incompatible format: Expected float item size");

			if (info1.shape[2] != (ssize_t)desc->samples())
				throw std::runtime_error("Incompatible descriptor: Expected sample size is not equal channel count");

			// rgbOut
			float* mem = new float[info1.shape[0] * info1.shape[1] * elems];
			tm.map(desc,
				   (float*)info1.ptr,
				   mem, elems,
				   info1.shape[0] * info1.shape[1]);

			py::capsule free_when_done(mem, [](void* f) {
				delete[] reinterpret_cast<float*>(f);
			});

			return py::array_t<float>(
				std::vector<size_t>({ (size_t)info1.shape[1], (size_t)info1.shape[0], elems }),
				std::vector<size_t>({ info1.shape[1] * elems * sizeof(float),
									  elems * sizeof(float),
									  sizeof(float) }),
				mem,
				free_when_done);
		})
		.def("mapOnlyMapper", [](ToneMapper& tm, Array rgbIn, size_t elems) {
			// specIn
			py::buffer_info info1 = rgbIn.request();
			if (info1.format != py::format_descriptor<float>::format())
				throw std::runtime_error("Incompatible format: expected a float array!");

			if (info1.ndim != 3)
				throw std::runtime_error("Incompatible buffer dimension. Expected 2d");

			if (info1.itemsize != sizeof(float))
				throw std::runtime_error("Incompatible format: Expected float item size");

			if (info1.shape[2] != (ssize_t)elems)
				throw std::runtime_error("Incompatible shape: Expected RGB in inner most dimension");

			if (info1.strides[2] != sizeof(float))
				throw std::runtime_error("Incompatible format: Expected float stride in the inner most dimension");

			// rgbOut
			float* mem = new float[info1.shape[0] * info1.shape[1] * elems]; // Other way??
			tm.mapOnlyMapper((float*)info1.ptr, mem, elems, info1.shape[0] * info1.shape[1]);

			py::capsule free_when_done(mem, [](void* f) {
				delete[] reinterpret_cast<float*>(f);
			});

			return py::array_t<float>(
				std::vector<size_t>({ (size_t)info1.shape[0], (size_t)info1.shape[1], elems }),
				std::vector<size_t>({ info1.shape[1] * elems * sizeof(float),
									  elems * sizeof(float),
									  sizeof(float) }),
				mem,
				free_when_done);
		})
		.def_property("colorMode", &ToneMapper::colorMode, &ToneMapper::setColorMode)
		.def_property("gammaMode", &ToneMapper::gammaMode, &ToneMapper::setGammaMode)
		.def_property("mapperMode", &ToneMapper::mapperMode, &ToneMapper::setMapperMode);

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
} // namespace PRPY