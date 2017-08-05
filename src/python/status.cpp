#include "renderer/RenderStatus.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
class RenderStatusWrap : public RenderStatus {
public:
	py::object getField_Py(const std::string& unique_name) const
	{
		Variant var = getField(unique_name);
		switch (var.type()) {
		case Variant::T_Bool:
			return py::cast(var.getBool());
		case Variant::T_Int:
			return py::cast(var.getInt());
		case Variant::T_UInt:
			return py::cast(var.getUInt());
		case Variant::T_Number:
			return py::cast(var.getNumber());
		case Variant::T_String:
			return py::cast(var.getString());
		}
		return py::object();
	}
};

void setup_status(py::module& m)
{
	py::class_<RenderStatusWrap>(m, "RenderStatus")
		.def_property("percentage", &RenderStatus::percentage, &RenderStatus::setPercentage)
		.def("__getitem__", &RenderStatusWrap::getField_Py)
		.def("__setitem__", &RenderStatus::setField)
		.def("__contains__", &RenderStatus::hasField);
}
}