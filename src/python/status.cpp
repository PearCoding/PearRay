#include "renderer/RenderStatus.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
PR_NO_SANITIZE_ADDRESS
void setup_status(py::module& m)
{
	py::class_<RenderStatus>(m, "RenderStatus")
		.def_property_readonly("percentage", &RenderStatus::percentage)
		.def("__getitem__", [](const RenderStatus& s, const std::string& unique_name) {
			const auto var = s.getField(unique_name);
			py::object obj;
			std::visit([&](auto&& v) { obj = py::cast(v); }, var);
			return obj;
		})
		.def("__contains__", &RenderStatus::hasField);
}
} // namespace PRPY