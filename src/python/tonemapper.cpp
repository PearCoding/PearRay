#include "spectral/ToneMapper.h"
#include "buffer/ColorBuffer.h"
#include "buffer/FrameBuffer.h"

#include "pypearray.h"
#include <pybind11/numpy.h>

using namespace PR;
namespace PRPY {
static void check3DBuffer(const py::buffer_info& info, size_t width, size_t height)
{
	if (info.format != py::format_descriptor<float>::format())
		throw std::runtime_error("Incompatible format: expected a float array!");

	if (info.ndim != 3)
		throw std::runtime_error("Incompatible buffer dimension. Expected 3d");

	if (info.itemsize != sizeof(float))
		throw std::runtime_error("Incompatible format: Expected float item size");

	if ((size_t)info.shape[0] != height
		|| (size_t)info.shape[1] != width)
		throw std::runtime_error("Incompatible shape: Outermost dimensions do not equal tone mapper");
}

static void checkColorBuffer(const py::buffer_info& info, size_t width, size_t height)
{
	check3DBuffer(info, width, height);
	if (info.shape[2] != (ssize_t)3)
		throw std::runtime_error("Incompatible descriptor: Expected sample size is not equal channel count");
}

static void checkWeightBuffer(const py::buffer_info& info, size_t width, size_t height)
{
	check3DBuffer(info, width, height);
	if (info.shape[2] != (ssize_t)1)
		throw std::runtime_error("Incompatible descriptor: Expected only one channel");
}

PR_NO_SANITIZE_ADDRESS
void setup_tonemapper(py::module& m)
{
	typedef py::array_t<float, py::array::c_style | py::array::forcecast> Array;

	py::enum_<ColorBufferMode>(m, "ColorBufferMode")
		.value("RGB", ColorBufferMode::RGB)
		.value("RGBA", ColorBufferMode::RGBA);

	py::class_<ColorBuffer>(m, "ColorBuffer", py::buffer_protocol())
		.def(py::init<uint32, uint32, ColorBufferMode>(),
			 py::arg("width"), py::arg("height"), py::arg("mode") = ColorBufferMode::RGBA)
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
			checkColorBuffer(info1, c.width(), c.height());
			c.map(mapper, (float*)info1.ptr);
		})
		.def("mapWeighted", [](ColorBuffer& c, const ToneMapper& mapper, Array specIn, Array weightIn) {
			py::buffer_info info1 = specIn.request();
			checkColorBuffer(info1, c.width(), c.height());
			py::buffer_info info2 = weightIn.request();
			checkWeightBuffer(info2, c.width(), c.height());

			c.map(mapper, (float*)info1.ptr, (float*)info2.ptr);
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
			checkColorBuffer(info1, info1.shape[1], info1.shape[0]);

			// rgbOut
			float* mem = new float[info1.shape[0] * info1.shape[1] * elems];
			tm.map((float*)info1.ptr, nullptr,
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
		.def("mapWeighted", [](ToneMapper& tm, Array specIn, Array weightIn, size_t elems) {
			// specIn
			py::buffer_info info1 = specIn.request();
			checkColorBuffer(info1, info1.shape[1], info1.shape[0]);

			// weightIn
			py::buffer_info info2 = weightIn.request();
			checkWeightBuffer(info2, info1.shape[1], info1.shape[0]);

			// rgbOut
			float* mem = new float[info1.shape[0] * info1.shape[1] * elems];
			tm.map((float*)info1.ptr, (float*)info2.ptr,
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
		.value("SRGB", ToneColorMode::SRGB)
		.value("XYZ", ToneColorMode::XYZ)
		.value("XYZ_NORM", ToneColorMode::XYZNorm)
		.value("LUMINANCE", ToneColorMode::Luminance);
}
} // namespace PRPY