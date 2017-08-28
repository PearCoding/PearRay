#include "renderer/OutputMap.h"
#include "shader/ShaderClosure.h"

#include "pypearray.h"
#include <pybind11/numpy.h>

using namespace PR;
namespace PRPY {
void setup_output(py::module& m)
{
	/* TODO: Add buffer protocol for vector3f */
	py::class_<FrameBuffer3D, std::shared_ptr<FrameBuffer3D>>(m, "FrameBuffer3D")
		.def(py::init<const Eigen::Vector3f&, bool>(), py::arg("clear_value"), py::arg("never_clear") = false)
		.def_property("neverClear", &FrameBuffer3D::isNeverCleared, &FrameBuffer3D::setNeverClear)
		.def("setFragment", &FrameBuffer3D::setFragment)
		.def("getFragment", &FrameBuffer3D::getFragment)
		.def("setFragmentBounded", &FrameBuffer3D::setFragmentBounded)
		.def("getFragmentBounded", &FrameBuffer3D::getFragmentBounded)
		.def("clear", &FrameBuffer3D::clear)
		.def("fill", &FrameBuffer3D::fill);

	py::class_<FrameBuffer1D, std::shared_ptr<FrameBuffer1D>>(m, "FrameBuffer1D", py::buffer_protocol())
		.def(py::init<const float&, bool>(), py::arg("clear_value") = float(), py::arg("never_clear") = false)
		.def_buffer([](FrameBuffer1D& s) -> py::buffer_info { // Allow buffer use
			return py::buffer_info(
				s.ptr(),
				sizeof(float),
				py::format_descriptor<float>::format(),
				2,
				std::vector<size_t>({ s.height(),
									  s.width() }),
				std::vector<size_t>({ s.width() * sizeof(float),
									  sizeof(float) }));
		})
		.def_property("neverClear", &FrameBuffer1D::isNeverCleared, &FrameBuffer1D::setNeverClear)
		.def("setFragment", &FrameBuffer1D::setFragment)
		.def("getFragment", &FrameBuffer1D::getFragment)
		.def("setFragmentBounded", &FrameBuffer1D::setFragmentBounded)
		.def("getFragmentBounded", &FrameBuffer1D::getFragmentBounded)
		.def("clear", &FrameBuffer1D::clear)
		.def("fill", &FrameBuffer1D::fill);

	py::class_<FrameBufferCounter, std::shared_ptr<FrameBufferCounter>>(m, "FrameBufferCounter", py::buffer_protocol())
		.def(py::init<const uint64&, bool>(), py::arg("clear_value") = uint64(), py::arg("never_clear") = false)
		.def_buffer([](FrameBufferCounter& s) -> py::buffer_info { // Allow buffer use
			return py::buffer_info(
				s.ptr(),
				sizeof(uint64),
				py::format_descriptor<uint64>::format(),
				2,
				std::vector<size_t>({ s.height(),
									  s.width() }),
				std::vector<size_t>({ s.width() * sizeof(uint64),
									  sizeof(uint64) }));
		})
		.def_property("neverClear", &FrameBufferCounter::isNeverCleared, &FrameBufferCounter::setNeverClear)
		.def("setFragment", &FrameBufferCounter::setFragment)
		.def("getFragment", &FrameBufferCounter::getFragment)
		.def("setFragmentBounded", &FrameBufferCounter::setFragmentBounded)
		.def("getFragmentBounded", &FrameBufferCounter::getFragmentBounded)
		.def("clear", &FrameBufferCounter::clear)
		.def("fill", &FrameBufferCounter::fill);

	py::class_<FrameBufferSpectrum, std::shared_ptr<FrameBufferSpectrum>>(m, "FrameBufferSpectrum", py::buffer_protocol())
		.def(py::init<const Spectrum&, bool>(), py::arg("clear_value") = Spectrum(), py::arg("never_clear") = false)
		.def_buffer([](FrameBufferSpectrum& s) -> py::buffer_info { // Allow buffer use
			return py::buffer_info(
				s.ptr(),
				sizeof(float),
				py::format_descriptor<float>::format(),
				3,
				std::vector<size_t>({ s.height(),
									  s.width(),
									  Spectrum::SAMPLING_COUNT }),
				std::vector<size_t>({ s.width() * Spectrum::SAMPLING_COUNT * sizeof(float),
									  Spectrum::SAMPLING_COUNT * sizeof(float),
									  sizeof(float) }));
		})
		.def_property("neverClear", &FrameBufferSpectrum::isNeverCleared, &FrameBufferSpectrum::setNeverClear)
		.def("setFragment", &FrameBufferSpectrum::setFragment)
		.def("getFragment", &FrameBufferSpectrum::getFragment)
		.def("setFragmentBounded", &FrameBufferSpectrum::setFragmentBounded)
		.def("getFragmentBounded", &FrameBufferSpectrum::getFragmentBounded)
		.def("clear", &FrameBufferSpectrum::clear)
		.def("fill", &FrameBufferSpectrum::fill);

	auto scope = py::class_<OutputMap>(m, "OutputMap");
	scope.def("clear", &OutputMap::clear)
		.def("pushFragment", &OutputMap::pushFragment)
		.def("fragment", &OutputMap::getFragment)
		.def("setSampleCount", &OutputMap::setSampleCount)
		.def("sampleCount", &OutputMap::getSampleCount)
		.def("isPixelFinished", &OutputMap::isPixelFinished)
		.def_property_readonly("finishedPixelCount", &OutputMap::finishedPixelCount)
		.def("channel",
			 (const std::shared_ptr<FrameBuffer1D>& (OutputMap::*)(OutputMap::Variable1D) const) & OutputMap::getChannel)
		.def("channel",
			 (const std::shared_ptr<FrameBuffer3D>& (OutputMap::*)(OutputMap::Variable3D) const) & OutputMap::getChannel)
		.def("channel",
			 (const std::shared_ptr<FrameBufferCounter>& (OutputMap::*)(OutputMap::VariableCounter) const) & OutputMap::getChannel)
		.def_property_readonly("spectral", &OutputMap::getSpectralChannel)
		.def("registerChannel",
			 (void (OutputMap::*)(OutputMap::Variable1D, const std::shared_ptr<FrameBuffer1D>&)) & OutputMap::registerChannel)
		.def("registerChannel",
			 (void (OutputMap::*)(OutputMap::Variable3D, const std::shared_ptr<FrameBuffer3D>&)) & OutputMap::registerChannel)
		.def("registerChannel",
			 (void (OutputMap::*)(OutputMap::VariableCounter, const std::shared_ptr<FrameBufferCounter>&)) & OutputMap::registerChannel)
		.def("registerCustomChannel",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<FrameBuffer1D>&)) & OutputMap::registerCustomChannel)
		.def("registerCustomChannel",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<FrameBuffer3D>&)) & OutputMap::registerCustomChannel)
		.def("registerCustomChannel",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<FrameBufferCounter>&)) & OutputMap::registerCustomChannel)
		.def("registerCustomChannel",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<FrameBufferSpectrum>&)) & OutputMap::registerCustomChannel);

	py::enum_<OutputMap::Variable3D>(scope, "Variable3D")
		.value("POSITION", OutputMap::V_Position)
		.value("NORMAL", OutputMap::V_Normal)
		.value("NORMALG", OutputMap::V_NormalG)
		.value("TANGENT", OutputMap::V_Tangent)
		.value("BINORMAL", OutputMap::V_Bitangent)
		.value("VIEW", OutputMap::V_View)
		.value("UVW", OutputMap::V_UVW)
		.value("DPDU", OutputMap::V_DPDU)
		.value("DPDV", OutputMap::V_DPDV)
		.value("DPDW", OutputMap::V_DPDW)
		.value("DPDT", OutputMap::V_DPDT);

	py::enum_<OutputMap::Variable1D>(scope, "Variable1D")
		.value("DEPTH", OutputMap::V_Depth)
		.value("TIME", OutputMap::V_Time)
		.value("MATERIAL", OutputMap::V_Material);

	py::enum_<OutputMap::VariableCounter>(scope, "VariableCounter")
		.value("ID", OutputMap::V_ID)
		.value("SAMPLES", OutputMap::V_Samples);
}
}