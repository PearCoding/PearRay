#include "pypearray.h"

#include <Eigen/Geometry>
#include <pybind11/operators.h>

using namespace PR;
namespace PRPY {

void setup_math(py::module& m)
{
	typedef Eigen::Quaternion<float, 0> Quat;
	typedef Eigen::QuaternionBase<Quat> QuatBase;

	py::class_<QuatBase>(m, "QuaternionBasef")
		.def_static("Identity", &QuatBase::Identity)

		.def(py::self * py::self)
		.def(py::self *= py::self)

		.def("angularDistance", &QuatBase::angularDistance<Quat>)
		.def("conjugate", &QuatBase::conjugate)
		.def("dot", &QuatBase::dot<Quat>)
		.def("inverse", &QuatBase::inverse)
		.def("norm", &QuatBase::norm)
		.def("squaredNorm", &QuatBase::squaredNorm)
		.def("normalize", &QuatBase::normalize)
		.def("normalized", &QuatBase::normalized)
		.def("toRotationMatrix", &QuatBase::toRotationMatrix)
		.def("slerp", &QuatBase::slerp<Quat>)

		.def_property("w", [](const QuatBase& o) { return o.w(); }, [](QuatBase& o, float v) { o.w() = v; })
		.def_property("x", [](const QuatBase& o) { return o.x(); }, [](QuatBase& o, float v) { o.x() = v; })
		.def_property("y", [](const QuatBase& o) { return o.y(); }, [](QuatBase& o, float v) { o.y() = v; })
		.def_property("z", [](const QuatBase& o) { return o.z(); }, [](QuatBase& o, float v) { o.z() = v; })

		.def("__getitem__", [](const QuatBase& o, size_t i) {
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
		.def("__setitem__", [](QuatBase& o, size_t i, float v) {
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

	py::class_<Quat, QuatBase>(m, "Quaternionf")
		.def(py::init<>())
		.def(py::init<float, float, float, float>())
		.def_static("UnitRandom", &Eigen::Quaternionf::UnitRandom);
	//.def_static("FromTwoVectors", &Eigen::Quaternionf::FromTwoVectors<Eigen::Matrix<float,3,1>, Eigen::Matrix<float,3,1>>)
}
}