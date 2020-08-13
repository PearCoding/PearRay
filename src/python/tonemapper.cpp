#include "spectral/ToneMapper.h"
#include "buffer/ColorBuffer.h"
#include "buffer/FrameBuffer.h"

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
					   Array specIn) {
			py::buffer_info info1 = specIn.request();
			if (info1.format != py::format_descriptor<float>::format())
				throw std::runtime_error("Incompatible format: expected a float array!");

			if (info1.ndim != 3)
				throw std::runtime_error("Incompatible buffer dimension. Expected 3d");

			if (info1.itemsize != sizeof(float))
				throw std::runtime_error("Incompatible format: Expected float item size");

			if (info1.shape[2] != (ssize_t)3)
				throw std::runtime_error("Incompatible descriptor: Expected sample size is not equal channel count");

			if ((size_t)info1.shape[0] != c.height()
				|| (size_t)info1.shape[1] != c.width())
				throw std::runtime_error("Incompatible shape: Outermost dimensions do not equal tone mapper");

			c.map(mapper, (float*)info1.ptr);
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
					   Array specIn, size_t elems) {
			// specIn
			py::buffer_info info1 = specIn.request();
			if (info1.format != py::format_descriptor<float>::format())
				throw std::runtime_error("Incompatible format: expected a float array!");

			if (info1.ndim != 3)
				throw std::runtime_error("Incompatible buffer dimension. Expected 3d");

			if (info1.itemsize != sizeof(float))
				throw std::runtime_error("Incompatible format: Expected float item size");

			if (info1.shape[2] != 3)
				throw std::runtime_error("Incompatible descriptor: Expected sample size is not equal channel count");

			// rgbOut
			float* mem = new float[info1.shape[0] * info1.shape[1] * elems];
			tm.map((float*)info1.ptr,
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
		.def_property("colorMode", &ToneMapper::colorMode, &ToneMapper::setColorMode)
		.def_property("scale", &ToneMapper::scale, &ToneMapper::setScale);

	py::enum_<ToneColorMode>(m, "ToneColorMode")
		.value("SRGB", TCM_SRGB)
		.value("XYZ", TCM_XYZ)
		.value("XYZ_NORM", TCM_XYZ_NORM)
		.value("LUMINANCE", TCM_LUMINANCE);
}
} // namespace PRPY