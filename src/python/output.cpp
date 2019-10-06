#include "renderer/OutputBuffer.h"
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
		.def("setFragment", py::overload_cast<const Vector2i&, size_t, const uint64&>(&FrameBufferUInt64::setFragment))
		.def("getFragment", py::overload_cast<const Vector2i&, size_t>(&FrameBufferUInt64::getFragment))
		.def("setFragmentBounded", py::overload_cast<const Vector2i&, size_t, const uint64&>(&FrameBufferUInt64::setFragmentBounded))
		.def("getFragmentBounded", py::overload_cast<const Vector2i&, size_t>(&FrameBufferUInt64::getFragmentBounded))
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
		.def("setFragment", py::overload_cast<const Vector2i&, size_t, const float&>(&FrameBufferFloat::setFragment))
		.def("setFragmentSpectrum", [](FrameBufferFloat& f, const Vector2i& i, const Spectrum& s) { f.setFragmentSpectrum(i, s); })
		.def("getFragment", py::overload_cast<const Vector2i&, size_t>(&FrameBufferFloat::getFragment))
		.def("setFragmentBounded", py::overload_cast<const Vector2i&, size_t, const float&>(&FrameBufferFloat::setFragmentBounded))
		.def("setFragmentBoundedSpectrum", [](FrameBufferFloat& f, const Vector2i& i, const Spectrum& s) { f.setFragmentBoundedSpectrum(i, s); })
		.def("getFragmentBounded", py::overload_cast<const Vector2i&, size_t>(&FrameBufferFloat::getFragmentBounded))
		.def("addFragmentSpectrum", [](FrameBufferFloat& f, const Vector2i& i, const Spectrum& s) { f.addFragmentSpectrum(i, s); })
		.def("addFragmentBoundedSpectrum", [](FrameBufferFloat& f, const Vector2i& i, const Spectrum& s) { f.addFragmentBoundedSpectrum(i, s); })
		.def("clear", &FrameBufferFloat::clear)
		.def("fill", &FrameBufferFloat::fill)
		.def_property_readonly("channels", &FrameBufferFloat::channels);

	auto scope = py::class_<OutputBuffer>(m, "OutputBuffer");
	scope.def("clear", &OutputBuffer::clear)
		.def("fragment", &OutputBuffer::getFragment)
		.def("sampleCount", &OutputBuffer::getSampleCount)
		.def("channel",
			 (std::shared_ptr<FrameBufferFloat>(OutputBuffer::*)(OutputBuffer::Variable1D) const) & OutputBuffer::getChannel)
		.def("channel",
			 (std::shared_ptr<FrameBufferFloat>(OutputBuffer::*)(OutputBuffer::Variable3D) const) & OutputBuffer::getChannel)
		.def("channel",
			 (std::shared_ptr<FrameBufferUInt64>(OutputBuffer::*)(OutputBuffer::VariableCounter) const) & OutputBuffer::getChannel)
		.def_property_readonly("spectral", &OutputBuffer::getSpectralChannel)
		.def("registerChannel",
			 (void (OutputBuffer::*)(OutputBuffer::Variable1D, const std::shared_ptr<FrameBufferFloat>&)) & OutputBuffer::registerChannel)
		.def("registerChannel",
			 (void (OutputBuffer::*)(OutputBuffer::Variable3D, const std::shared_ptr<FrameBufferFloat>&)) & OutputBuffer::registerChannel)
		.def("registerChannel",
			 (void (OutputBuffer::*)(OutputBuffer::VariableCounter, const std::shared_ptr<FrameBufferUInt64>&)) & OutputBuffer::registerChannel)
		.def("registerCustomChannel_1D",
			 (void (OutputBuffer::*)(const std::string&, const std::shared_ptr<FrameBufferFloat>&)) & OutputBuffer::registerCustomChannel_1D)
		.def("registerCustomChannel_3D",
			 (void (OutputBuffer::*)(const std::string&, const std::shared_ptr<FrameBufferFloat>&)) & OutputBuffer::registerCustomChannel_3D)
		.def("registerCustomChannel_Counter",
			 (void (OutputBuffer::*)(const std::string&, const std::shared_ptr<FrameBufferUInt64>&)) & OutputBuffer::registerCustomChannel_Counter)
		.def("registerCustomChannel_Spectral",
			 (void (OutputBuffer::*)(const std::string&, const std::shared_ptr<FrameBufferFloat>&)) & OutputBuffer::registerCustomChannel_Spectral);

	py::enum_<OutputBuffer::Variable3D>(scope, "Variable3D")
		.value("POSITION", OutputBuffer::V_Position)
		.value("NORMAL", OutputBuffer::V_Normal)
		.value("NORMALG", OutputBuffer::V_NormalG)
		.value("TANGENT", OutputBuffer::V_Tangent)
		.value("BINORMAL", OutputBuffer::V_Bitangent)
		.value("VIEW", OutputBuffer::V_View)
		.value("UVW", OutputBuffer::V_UVW)
		.value("DPDU", OutputBuffer::V_DPDU)
		.value("DPDV", OutputBuffer::V_DPDV)
		.value("DPDW", OutputBuffer::V_DPDW)
		.value("DPDT", OutputBuffer::V_DPDT);

	py::enum_<OutputBuffer::Variable1D>(scope, "Variable1D")
		.value("DEPTH", OutputBuffer::V_Depth)
		.value("TIME", OutputBuffer::V_Time)
		.value("MATERIAL", OutputBuffer::V_Material);

	py::enum_<OutputBuffer::VariableCounter>(scope, "VariableCounter")
		.value("ID", OutputBuffer::V_ID)
		.value("SAMPLES", OutputBuffer::V_Samples);
}
} // namespace PRPY