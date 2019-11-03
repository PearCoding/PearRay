#include "renderer/RenderStatus.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
void setup_status(py::module& m)
{
	py::class_<RenderStatus>(m, "RenderStatus")
		.def_property_readonly("percentage", &RenderStatus::percentage)
		.def("__getitem__", [](const RenderStatus& s, const std::string& unique_name) {
			Variant var = s.getField(unique_name);
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
		})
		.def("__contains__", &RenderStatus::hasField);
}
} // namespace PRPY