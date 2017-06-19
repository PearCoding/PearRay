#include "renderer/OutputMap.h"
#include "shader/ShaderClosure.h"
#include <boost/python.hpp>

#include "npmath.h"

using namespace PR;
namespace bpy = boost::python;
namespace np  = boost::python::numpy;

namespace PRPY {
class Output3DWrap : public Output3D, public bpy::wrapper<Output3D> {
public:
	Output3DWrap(RenderContext* renderer, const np::ndarray& clear_value, bool never_clear = false)
		: Output3D(renderer, vec3FromPython(clear_value), never_clear)
	{
	}

	void setFragment_Py(const np::ndarray& p, const np::ndarray& v)
	{
		setFragment(ivec2FromPython(p), vec3FromPython(v));
	}

	np::ndarray getFragment_Py(const np::ndarray& p) const
	{
		return vec3ToPython(getFragment(ivec2FromPython(p)));
	}

	void setFragmentBounded_Py(const np::ndarray& p, const np::ndarray& v)
	{
		setFragmentBounded(ivec2FromPython(p), vec3FromPython(v));
	}

	np::ndarray getFragmentBounded_Py(const np::ndarray& p) const
	{
		return vec3ToPython(getFragmentBounded(ivec2FromPython(p)));
	}

	void fill_Py(const np::ndarray& v)
	{
		fill(vec3FromPython(v));
	}
};

class Output1DWrap : public Output1D, public bpy::wrapper<Output1D> {
public:
	Output1DWrap(RenderContext* renderer, float clear_value = 0.0f, bool never_clear = false)
		: Output1D(renderer, clear_value, never_clear)
	{
	}

	void setFragment_Py(const np::ndarray& p, float f)
	{
		setFragment(ivec2FromPython(p), f);
	}

	float getFragment_Py(const np::ndarray& p) const
	{
		return getFragment(ivec2FromPython(p));
	}

	void setFragmentBounded_Py(const np::ndarray& p, float f)
	{
		setFragmentBounded(ivec2FromPython(p), f);
	}

	float getFragmentBounded_Py(const np::ndarray& p) const
	{
		return getFragmentBounded(ivec2FromPython(p));
	}
};

class OutputCounterWrap : public OutputCounter, public bpy::wrapper<OutputCounter> {
public:
	OutputCounterWrap(RenderContext* renderer, uint64 clear_value = 0, bool never_clear = false)
		: OutputCounter(renderer, clear_value, never_clear)
	{
	}

	void setFragment_Py(const np::ndarray& p, uint64 f)
	{
		setFragment(ivec2FromPython(p), f);
	}

	uint64 getFragment_Py(const np::ndarray& p) const
	{
		return getFragment(ivec2FromPython(p));
	}

	void setFragmentBounded_Py(const np::ndarray& p, uint64 f)
	{
		setFragmentBounded(ivec2FromPython(p), f);
	}

	uint64 getFragmentBounded_Py(const np::ndarray& p) const
	{
		return getFragmentBounded(ivec2FromPython(p));
	}
};

class OutputSpectralWrap : public OutputSpectral, public bpy::wrapper<OutputSpectral> {
public:
	OutputSpectralWrap(RenderContext* renderer, const Spectrum& clear_value = Spectrum(), bool never_clear = false)
		: OutputSpectral(renderer, clear_value, never_clear)
	{
	}

	void setFragment_Py(const np::ndarray& p, const Spectrum& v)
	{
		setFragment(ivec2FromPython(p), v);
	}

	Spectrum getFragment_Py(const np::ndarray& p) const
	{
		return getFragment(ivec2FromPython(p));
	}

	void setFragmentBounded_Py(const np::ndarray& p, const Spectrum& v)
	{
		setFragmentBounded(ivec2FromPython(p), v);
	}

	Spectrum getFragmentBounded_Py(const np::ndarray& p) const
	{
		return getFragmentBounded(ivec2FromPython(p));
	}
};

void setup_output()
{
	// TODO: Add ptr() for bpy::object?
	bpy::class_<Output3DWrap, std::shared_ptr<Output3DWrap>, boost::noncopyable>("Output3D", bpy::init<RenderContext*, const np::ndarray&, bpy::optional<bool>>())
		.add_property("neverClear", &Output3DWrap::isNeverCleared, &Output3D::setNeverClear)
		.def("setFragment", &Output3DWrap::setFragment_Py)
		.def("getFragment", &Output3DWrap::getFragment_Py)
		.def("setFragmentBounded", &Output3DWrap::setFragmentBounded_Py)
		.def("getFragmentBounded", &Output3DWrap::getFragmentBounded_Py)
		.def("clear", &Output3D::clear)
		.def("fill", &Output3DWrap::fill_Py);

	bpy::class_<Output1DWrap, std::shared_ptr<Output1DWrap>, boost::noncopyable>("Output1D", bpy::init<RenderContext*, bpy::optional<float, bool>>())
		.add_property("neverClear", &Output1D::isNeverCleared, &Output1D::setNeverClear)
		.def("setFragment", &Output1DWrap::setFragment_Py)
		.def("getFragment", &Output1DWrap::getFragment_Py)
		.def("setFragmentBounded", &Output1DWrap::setFragmentBounded_Py)
		.def("getFragmentBounded", &Output1DWrap::getFragmentBounded_Py)
		.def("clear", &Output1D::clear)
		.def("fill", &Output1D::fill);

	bpy::class_<OutputCounterWrap, std::shared_ptr<OutputCounterWrap>, boost::noncopyable>("OutputCounter", bpy::init<RenderContext*, bpy::optional<uint64, bool>>())
		.add_property("neverClear", &OutputCounter::isNeverCleared, &OutputCounter::setNeverClear)
		.def("setFragment", &OutputCounterWrap::setFragment_Py)
		.def("getFragment", &OutputCounterWrap::getFragment_Py)
		.def("setFragmentBounded", &OutputCounterWrap::setFragmentBounded_Py)
		.def("getFragmentBounded", &OutputCounterWrap::getFragmentBounded_Py)
		.def("clear", &OutputCounter::clear)
		.def("fill", &OutputCounter::fill);

	bpy::class_<OutputSpectralWrap, std::shared_ptr<OutputSpectralWrap>, boost::noncopyable>("OutputSpectral", bpy::init<RenderContext*, bpy::optional<Spectrum, bool>>())
		.add_property("neverClear", &OutputSpectral::isNeverCleared, &OutputSpectral::setNeverClear)
		.def("setFragment", &OutputSpectralWrap::setFragment_Py)
		.def("getFragment", &OutputSpectralWrap::getFragment_Py)
		.def("setFragmentBounded", &OutputSpectralWrap::setFragmentBounded_Py)
		.def("getFragmentBounded", &OutputSpectralWrap::getFragmentBounded_Py)
		.def("clear", &OutputSpectral::clear)
		.def("fill", &OutputSpectral::fill);

	bpy::register_ptr_to_python<std::shared_ptr<Output3D>>();
	bpy::register_ptr_to_python<std::shared_ptr<Output1D>>();
	bpy::register_ptr_to_python<std::shared_ptr<OutputCounter>>();
	bpy::register_ptr_to_python<std::shared_ptr<OutputSpectral>>();

	{
		bpy::scope scope = bpy::class_<OutputMap, boost::noncopyable>("OutputMap", bpy::no_init)
							   .def("clear", &OutputMap::clear)
							   .def("pushFragment", &OutputMap::pushFragment)
							   .def("fragment", &OutputMap::getFragment)
							   .def("setSampleCount", &OutputMap::setSampleCount)
							   .def("sampleCount", &OutputMap::getSampleCount)
							   .def("isPixelFinished", &OutputMap::isPixelFinished)
							   .add_property("finishedPixelCount", &OutputMap::finishedPixelCount)
							   .def("channel",
									(const std::shared_ptr<Output1D>& (OutputMap::*)(OutputMap::Variable1D) const) & OutputMap::getChannel,
									bpy::return_value_policy<bpy::copy_const_reference>())
							   .def("channel",
									(const std::shared_ptr<Output3D>& (OutputMap::*)(OutputMap::Variable3D) const) & OutputMap::getChannel,
									bpy::return_value_policy<bpy::copy_const_reference>())
							   .def("channel",
									(const std::shared_ptr<OutputCounter>& (OutputMap::*)(OutputMap::VariableCounter) const) & OutputMap::getChannel,
									bpy::return_value_policy<bpy::copy_const_reference>())
							   .add_property("spectral", bpy::make_function(&OutputMap::getSpectralChannel, bpy::return_internal_reference<>()))
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

		bpy::enum_<OutputMap::Variable3D>("Variable3D")
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
			.value("DPDX", OutputMap::V_DPDX)
			.value("DPDY", OutputMap::V_DPDY)
			.value("DPDZ", OutputMap::V_DPDZ)
			.value("DPDT", OutputMap::V_DPDT);

		bpy::enum_<OutputMap::Variable1D>("Variable1D")
			.value("DEPTH", OutputMap::V_Depth)
			.value("TIME", OutputMap::V_Time)
			.value("MATERIAL", OutputMap::V_Material);

		bpy::enum_<OutputMap::VariableCounter>("VariableCounter")
			.value("ID", OutputMap::V_ID)
			.value("SAMPLES", OutputMap::V_Samples);
	}
}
}