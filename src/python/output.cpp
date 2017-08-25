#include "renderer/OutputMap.h"
#include "shader/ShaderClosure.h"

#include "pypearray.h"
#include <pybind11/numpy.h>

using namespace PR;
namespace PRPY {
void setup_output(py::module& m)
{
	/* TODO: Add buffer protocol for vector3f */
	py::class_<Output3D, std::shared_ptr<Output3D>>(m, "Output3D")
		.def(py::init<const Eigen::Vector3f&, bool>(), py::arg("clear_value"), py::arg("never_clear") = false)
		.def_property("neverClear", &Output3D::isNeverCleared, &Output3D::setNeverClear)
		.def("setFragment", &Output3D::setFragment)
		.def("getFragment", &Output3D::getFragment)
		.def("setFragmentBounded", &Output3D::setFragmentBounded)
		.def("getFragmentBounded", &Output3D::getFragmentBounded)
		.def("clear", &Output3D::clear)
		.def("fill", &Output3D::fill);

	py::class_<Output1D, std::shared_ptr<Output1D>>(m, "Output1D", py::buffer_protocol())
		.def(py::init<const float&, bool>(), py::arg("clear_value") = float(), py::arg("never_clear") = false)
		.def_buffer([](Output1D& s) -> py::buffer_info { // Allow buffer use
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
		.def_property("neverClear", &Output1D::isNeverCleared, &Output1D::setNeverClear)
		.def("setFragment", &Output1D::setFragment)
		.def("getFragment", &Output1D::getFragment)
		.def("setFragmentBounded", &Output1D::setFragmentBounded)
		.def("getFragmentBounded", &Output1D::getFragmentBounded)
		.def("clear", &Output1D::clear)
		.def("fill", &Output1D::fill);

	py::class_<OutputCounter, std::shared_ptr<OutputCounter>>(m, "OutputCounter", py::buffer_protocol())
		.def(py::init<const uint64&, bool>(), py::arg("clear_value") = uint64(), py::arg("never_clear") = false)
		.def_buffer([](OutputCounter& s) -> py::buffer_info { // Allow buffer use
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
		.def_property("neverClear", &OutputCounter::isNeverCleared, &OutputCounter::setNeverClear)
		.def("setFragment", &OutputCounter::setFragment)
		.def("getFragment", &OutputCounter::getFragment)
		.def("setFragmentBounded", &OutputCounter::setFragmentBounded)
		.def("getFragmentBounded", &OutputCounter::getFragmentBounded)
		.def("clear", &OutputCounter::clear)
		.def("fill", &OutputCounter::fill);

	py::class_<OutputSpectral, std::shared_ptr<OutputSpectral>>(m, "OutputSpectral", py::buffer_protocol())
		.def(py::init<const Spectrum&, bool>(), py::arg("clear_value") = Spectrum(), py::arg("never_clear") = false)
		.def_buffer([](OutputSpectral& s) -> py::buffer_info { // Allow buffer use
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
		.def_property("neverClear", &OutputSpectral::isNeverCleared, &OutputSpectral::setNeverClear)
		.def("setFragment", &OutputSpectral::setFragment)
		.def("getFragment", &OutputSpectral::getFragment)
		.def("setFragmentBounded", &OutputSpectral::setFragmentBounded)
		.def("getFragmentBounded", &OutputSpectral::getFragmentBounded)
		.def("clear", &OutputSpectral::clear)
		.def("fill", &OutputSpectral::fill);

	auto scope = py::class_<OutputMap>(m, "OutputMap");
	scope.def("clear", &OutputMap::clear)
		.def("pushFragment", &OutputMap::pushFragment)
		.def("fragment", &OutputMap::getFragment)
		.def("setSampleCount", &OutputMap::setSampleCount)
		.def("sampleCount", &OutputMap::getSampleCount)
		.def("isPixelFinished", &OutputMap::isPixelFinished)
		.def_property_readonly("finishedPixelCount", &OutputMap::finishedPixelCount)
		.def("channel",
			 (const std::shared_ptr<Output1D>& (OutputMap::*)(OutputMap::Variable1D) const) & OutputMap::getChannel)
		.def("channel",
			 (const std::shared_ptr<Output3D>& (OutputMap::*)(OutputMap::Variable3D) const) & OutputMap::getChannel)
		.def("channel",
			 (const std::shared_ptr<OutputCounter>& (OutputMap::*)(OutputMap::VariableCounter) const) & OutputMap::getChannel)
		.def_property_readonly("spectral", &OutputMap::getSpectralChannel)
		.def("registerChannel",
			 (void (OutputMap::*)(OutputMap::Variable1D, const std::shared_ptr<Output1D>&)) & OutputMap::registerChannel)
		.def("registerChannel",
			 (void (OutputMap::*)(OutputMap::Variable3D, const std::shared_ptr<Output3D>&)) & OutputMap::registerChannel)
		.def("registerChannel",
			 (void (OutputMap::*)(OutputMap::VariableCounter, const std::shared_ptr<OutputCounter>&)) & OutputMap::registerChannel)
		.def("registerCustomChannel",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<Output1D>&)) & OutputMap::registerCustomChannel)
		.def("registerCustomChannel",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<Output3D>&)) & OutputMap::registerCustomChannel)
		.def("registerCustomChannel",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<OutputCounter>&)) & OutputMap::registerCustomChannel)
		.def("registerCustomChannel",
			 (void (OutputMap::*)(const std::string&, const std::shared_ptr<OutputSpectral>&)) & OutputMap::registerCustomChannel);

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