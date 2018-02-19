#include "renderer/OutputMap.h"
#include "shader/ShaderClosure.h"

#include "pypearray.h"
#include <pybind11/numpy.h>

using namespace PR;
namespace PRPY {
void setup_output(py::module& m)
{
	py::class_<FrameBufferUInt64, std::shared_ptr<FrameBufferUInt64>>(m, "FrameBufferUInt64", py::buffer_protocol())
		.def(py::init<const uint64&, bool>(), py::arg("clear_value") = uint64(), py::arg("never_clear") = false)
		.def_buffer([](FrameBufferUInt64& s) -> py::buffer_info { // Allow buffer use
			return py::buffer_info(
				s.ptr(),
				sizeof(uint64),
				py::format_descriptor<uint64>::format(),
				2,
				std::vector<size_t>({ s.height(),
									  s.width(),
									  s.channels() }),
				std::vector<size_t>({ s.width() * s.channels() * sizeof(uint64),
									  s.channels() * sizeof(uint64),
									  sizeof(uint64) }));
		})
		.def_property("neverClear", &FrameBufferUInt64::isNeverCleared, &FrameBufferUInt64::setNeverClear)
		.def("setFragment", py::overload_cast<const Eigen::Vector2i&, size_t, const uint64&>(&FrameBufferUInt64::setFragment))
		.def("getFragment", py::overload_cast<const Eigen::Vector2i&, size_t>(&FrameBufferUInt64::getFragment))
		.def("setFragmentBounded", py::overload_cast<const Eigen::Vector2i&, size_t, const uint64&>(&FrameBufferUInt64::setFragment))
		.def("getFragmentBounded", py::overload_cast<const Eigen::Vector2i&, size_t>(&FrameBufferUInt64::getFragment))
		.def("clear", &FrameBufferUInt64::clear)
		.def("fill", &FrameBufferUInt64::fill)
		.def_property_readonly("channels", &FrameBufferUInt64::channels);

	py::class_<FrameBufferFloat, std::shared_ptr<FrameBufferFloat>>(m, "FrameBufferFloat", py::buffer_protocol())
		.def(py::init<const float&, bool>(), py::arg("clear_value") = 0.0f, py::arg("never_clear") = false)
		.def_buffer([](FrameBufferFloat& s) -> py::buffer_info { // Allow buffer use
			return py::buffer_info(
				s.ptr(),
				sizeof(float),
				py::format_descriptor<float>::format(),
				3,
				std::vector<size_t>({ s.height(),
									  s.width(),
									  s.channels() }),
				std::vector<size_t>({ s.width() * s.channels() * sizeof(float),
									  s.channels() * sizeof(float),
									  sizeof(float) }));
		})
		.def_property("neverClear", &FrameBufferFloat::isNeverCleared, &FrameBufferFloat::setNeverClear)
		.def("setFragment", py::overload_cast<const Eigen::Vector2i&, size_t, const float&>(&FrameBufferFloat::setFragment))
		.def("setFragment", py::overload_cast<const Eigen::Vector2i&, const Spectrum&>(&FrameBufferFloat::setFragment))
		.def("getFragment", py::overload_cast<const Eigen::Vector2i&, size_t>(&FrameBufferFloat::getFragment))
		.def("setFragmentBounded", py::overload_cast<const Eigen::Vector2i&, size_t, const float&>(&FrameBufferFloat::setFragment))
		.def("setFragmentBounded", py::overload_cast<const Eigen::Vector2i&, const Spectrum&>(&FrameBufferFloat::setFragment))
		.def("getFragmentBounded", py::overload_cast<const Eigen::Vector2i&, size_t>(&FrameBufferFloat::getFragment))
		.def("clear", &FrameBufferFloat::clear)
		.def("fill", &FrameBufferFloat::fill)
		.def_property_readonly("channels", &FrameBufferFloat::channels);

	auto scope = py::class_<OutputMap>(m, "OutputMap");
	scope.def("clear", &OutputMap::clear)
		.def("fragment", &OutputMap::getFragment)
		.def("sampleCount", &OutputMap::getSampleCount)
		.def("channel",
			 (std::shared_ptr<FrameBufferFloat> (OutputMap::*)(OutputMap::Variable1D) const) & OutputMap::getChannel)
		.def("channel",
			 (std::shared_ptr<FrameBufferFloat> (OutputMap::*)(OutputMap::Variable3D) const) & OutputMap::getChannel)
		.def("channel",
			 (std::shared_ptr<FrameBufferUInt64> (OutputMap::*)(OutputMap::VariableCounter) const) & OutputMap::getChannel)
		.def_property_readonly("spectral", &OutputMap::getSpectralChannel)
		.def("registerChannel",
			 (void (OutputMap::*)(OutputMap::Variable1D, const std::shared_ptr<FrameBufferFloat>&)) & OutputMap::registerChannel)
		.def("registerChannel",
			 (void (OutputMap::*)(OutputMap::Variable3D, const std::shared_ptr<FrameBufferFloat>&)) & OutputMap::registerChannel)
		.def("registerChannel",
			 (void (OutputMap::*)(OutputMap::VariableCounter, const std::shared_ptr<FrameBufferUInt64>&)) & OutputMap::registerChannel)
		.def("registerCustomChannel_1D",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<FrameBufferFloat>&)) & OutputMap::registerCustomChannel_1D)
		.def("registerCustomChannel_3D",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<FrameBufferFloat>&)) & OutputMap::registerCustomChannel_3D)
		.def("registerCustomChannel_Counter",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<FrameBufferUInt64>&)) & OutputMap::registerCustomChannel_Counter)
		.def("registerCustomChannel_Spectral",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<FrameBufferFloat>&)) & OutputMap::registerCustomChannel_Spectral);

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
} // namespace PRPY