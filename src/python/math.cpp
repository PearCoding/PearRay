#include "pypearray.h"

#include <Eigen/Geometry>
#include <pybind11/operators.h>

using namespace PR;
namespace PRPY {

PR_NO_SANITIZE_ADDRESS
void setup_math(py::module& m)
{
	typedef Eigen::Quaternion<float, 0> Quat;

	py::class_<Quat>(m, "Quaternionf")
		.def_static("Identity", &Quat::Identity)

		.def(py::init<>())
		.def(py::init<float, float, float, float>())
		.def_static("UnitRandom", &Quat::UnitRandom)

		.def(py::self * py::self)
		.def(py::self *= py::self)

		.def("angularDistance", &Quat::angularDistance<Quat>)
		.def("conjugate", &Quat::conjugate)
		.def("dot", &Quat::dot<Quat>)
		.def("inverse", &Quat::inverse)
		.def("norm", &Quat::norm)
		.def("squaredNorm", &Quat::squaredNorm)
		.def("normalize", &Quat::normalize)
		.def("normalized", &Quat::normalized)
		.def("toRotationMatrix", &Quat::toRotationMatrix)
		.def("slerp", &Quat::slerp<Quat>)

		.def_property("w", [](const Quat& o) { return o.w(); }, [](Quat& o, float v) { o.w() = v; })
		.def_property("x", [](const Quat& o) { return o.x(); }, [](Quat& o, float v) { o.x() = v; })
		.def_property("y", [](const Quat& o) { return o.y(); }, [](Quat& o, float v) { o.y() = v; })
		.def_property("z", [](const Quat& o) { return o.z(); }, [](Quat& o, float v) { o.z() = v; })

		.def("__getitem__", [](const Quat& o, size_t i) {
			switch (i) {
			case 0:
				return o.w();
			case 1:
				return o.x();
			case 2:
				return o.y();
			case 3:
				return o.z();
			default:
				throw std::runtime_error("Quaternion access out of bound!");
				return 0.0f;
			}
		})
		.def("__setitem__", [](Quat& o, size_t i, float v) {
			switch (i) {
			case 0:
				o.w() = v;
				break;
			case 1:
				o.x() = v;
				break;
			case 2:
				o.y() = v;
				break;
			case 3:
				o.z() = v;
				break;
			default:
				throw std::runtime_error("Quaternion access out of bound!");
			}
		});
	//.def_static("FromTwoVectors", &Eigen::Quaternionf::FromTwoVectors<Eigen::Matrix<float,3,1>, Eigen::Matrix<float,3,1>>)
}
}