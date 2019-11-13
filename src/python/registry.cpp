#include "registry/Registry.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {

#define SET_METHOD_V1(T) \
	.def("set", [](Registry& s, const URI& uri, T v){s.set(uri, v); })
#define SET_METHOD_V2(T) \
	.def("setByGroup", [](Registry& s, RegistryGroup grp, const URI& uri, T v){s.setByGroup(grp, uri, v); })
#define SET_METHOD_V3(T) \
	.def("setForObject", [](Registry& s, RegistryGroup grp, uint32 id, const URI& uri, T v){s.setForObject(grp, id, uri, v); })

#define SET_METHOD(T) \
	SET_METHOD_V1(T) \
	SET_METHOD_V2(T) \
	SET_METHOD_V3(T)

#define GET_METHOD_V1(n, T) \
	.def(n, [](const Registry& s, const URI& uri, T v){return s.get<T>(uri, v);})
#define GET_METHOD_V2(n, T) \
	.def(n, [](const Registry& s, RegistryGroup grp, const URI& uri, T v){return s.getByGroup<T>(grp, uri, v);})
#define GET_METHOD_V3(n, T) \
	.def(n, [](const Registry& s, RegistryGroup grp, uint32 id, const URI& uri, T v){return s.getForObject<T>(grp, id, uri, v);})\
	.def(n, [](const Registry& s, RegistryGroup grp, uint32 id, const URI& uri, T v, bool fallback){return s.getForObject<T>(grp, id, uri, v, fallback);})

#define GET_METHOD(n1, n2, n3, T) \
	GET_METHOD_V1(n1, T) \
	GET_METHOD_V2(n2, T) \
	GET_METHOD_V3(n3, T)

PR_NO_SANITIZE_ADDRESS
void setup_registry(py::module& m)
{
	py::class_<Registry, std::shared_ptr<Registry>>(m, "Registry")
		.def("dump", &Registry::dump)
		.def("exists", &Registry::exists)
		.def("existsByGroup", &Registry::existsByGroup)
		.def("existsForObject", &Registry::existsForObject)
		SET_METHOD(float)
		SET_METHOD(uint64)
		SET_METHOD(int64)
		SET_METHOD(bool)
		SET_METHOD(const std::string&)
		GET_METHOD("getFloat", "getByGroupFloat", "getForObjectFloat", float)
		GET_METHOD("getUInt", "getByGroupUInt", "getForObjectUInt", uint64)
		GET_METHOD("getInt", "getByGroupInt", "getForObjectInt", int64)
		GET_METHOD("getBool", "getByGroupBool", "getForObjectBool", bool)
		GET_METHOD("getString", "getByGroupString", "getForObjectString", std::string);

	py::enum_<RegistryGroup>(m, "RegistryGroup")
		.value("NONE", RG_NONE)
		.value("GLOBAL", RG_GLOBAL)
		.value("MATERIAL", RG_MATERIAL)
		.value("EMISSION", RG_EMISSION)
		.value("ENTITY", RG_ENTITY)
		.value("CAMERA", RG_CAMERA)
		.value("INFINITELIGHT", RG_INFINITELIGHT)
		.value("RENDERER", RG_RENDERER)
		.value("INTEGRATOR", RG_INTEGRATOR);

	py::class_<URI>(m, "URI")
		.def(py::init<>())
		.def(py::init<std::string>())
		.def("setFromString", &URI::setFromString)
		.def("makeAbsolute", &URI::makeAbsolute)
		.def("__str__", &URI::str)
		.def_property("protocol", &URI::protocol, &URI::setProtocol)
		.def_property("host", &URI::host, &URI::setHost)
		.def_property("path", &URI::path, &URI::setPath)
		.def_property("query", &URI::query, &URI::setQuery)
		.def_property("fragment", &URI::fragment, &URI::setFragment)
		.def_property_readonly("authority", &URI::authority)
		.def_property_readonly("external", &URI::isExternal)
		.def_property_readonly("absolute", &URI::isAbsolute)
		.def_property_readonly("valid", &URI::isValid);

	py::implicitly_convertible<std::string, URI>();
}
}