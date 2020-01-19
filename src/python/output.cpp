#include "buffer/OutputBuffer.h"

#include "pypearray.h"
#include <pybind11/numpy.h>

using namespace PR;
namespace PRPY {
PR_NO_SANITIZE_ADDRESS
void setup_output(py::module& m)
{
	py::class_<FrameBufferUInt32, std::shared_ptr<FrameBufferUInt32>>(m, "FrameBufferUInt32", py::buffer_protocol())
		/*.def(py::init<size_t, size_t, size_t, const uint32&, bool>(), py::arg("clear_value") = uint32(), py::arg("never_clear") = false)*/
		.def_buffer([](FrameBufferUInt32& s) -> py::buffer_info { // Allow buffer use
			return py::buffer_info(
				s.ptr(),
				sizeof(uint32),
				py::format_descriptor<uint32>::format(),
				3,
				std::vector<Size1i>({ s.height(),
									  s.width(),
									  s.channels() }),
				std::vector<Size1i>({ s.heightBytePitch(),
									  s.widthBytePitch(),
									  s.channelBytePitch() }));
		})
		.def_property("neverClear", &FrameBufferUInt32::isNeverCleared, &FrameBufferUInt32::setNeverClear)
		.def("setFragment", py::overload_cast<const Point2i&, Size1i, const uint32&>(&FrameBufferUInt32::setFragment))
		.def("getFragment", py::overload_cast<const Point2i&, Size1i>(&FrameBufferUInt32::getFragment))
		.def("clear", &FrameBufferUInt32::clear)
		.def("fill", &FrameBufferUInt32::fill)
		.def_property_readonly("channels", &FrameBufferUInt32::channels);

	py::class_<FrameBufferFloat, std::shared_ptr<FrameBufferFloat>>(m, "FrameBufferFloat", py::buffer_protocol())
		.def_buffer([](FrameBufferFloat& s) -> py::buffer_info { // Allow buffer use
			return py::buffer_info(
				s.ptr(),
				sizeof(float),
				py::format_descriptor<float>::format(),
				3,
				std::vector<Size1i>({ s.height(),
									  s.width(),
									  s.channels() }),
				std::vector<Size1i>({ s.heightBytePitch(),
									  s.widthBytePitch(),
									  s.channelBytePitch() }));
		})
		.def_property("neverClear", &FrameBufferFloat::isNeverCleared, &FrameBufferFloat::setNeverClear)
		.def("setFragment", py::overload_cast<const Point2i&, Size1i, const float&>(&FrameBufferFloat::setFragment))
		.def("getFragment", py::overload_cast<const Point2i&, Size1i>(&FrameBufferFloat::getFragment))
		.def("clear", &FrameBufferFloat::clear)
		.def("fill", &FrameBufferFloat::fill)
		.def_property_readonly("channels", &FrameBufferFloat::channels);

	// Hiding OutputBufferData
	// TODO: Add whole interface
	auto scope = py::class_<OutputBuffer, std::shared_ptr<OutputBuffer>>(m, "OutputBuffer");
	scope.def("clear", &OutputBuffer::clear)
		.def_property_readonly("spectral", [](const OutputBuffer& buffer) { return buffer.data().getInternalChannel_Spectral(); });

	py::enum_<AOV3D>(scope, "AOV3D")
		.value("POSITION", AOV_Position)
		.value("NORMAL", AOV_Normal)
		.value("NORMALG", AOV_NormalG)
		.value("TANGENT", AOV_Tangent)
		.value("BINORMAL", AOV_Bitangent)
		.value("VIEW", AOV_View)
		.value("UVW", AOV_UVW)
		.value("DPDT", AOV_DPDT);

	py::enum_<AOV1D>(scope, "AOV1D")
		.value("DEPTH", AOV_Depth)
		.value("TIME", AOV_Time)
		.value("ENTITY_ID", AOV_EntityID)
		.value("MATERIAL_ID", AOV_MaterialID)
		.value("EMISSION_ID", AOV_EmissionID)
		.value("DISPLACE_ID", AOV_DisplaceID);

	py::enum_<AOVCounter>(scope, "AOVCounter")
		.value("FEEDBACK", AOV_Feedback)
		.value("SAMPLES", AOV_SampleCount);
}
} // namespace PRPY