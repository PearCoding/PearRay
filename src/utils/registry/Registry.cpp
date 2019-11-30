#include "Registry.h"

#include <boost/core/demangle.hpp>
#include <vector>

namespace PR {
static URI sNonePath("");
static URI sGlobalsPath("/globals");
static URI sMaterialsPath("/materials");
static URI sEmissionsPath("/emissions");
static URI sEntitiesPath("/entities");
static URI sCamerasPath("/cameras");
static URI sInfiniteLightsPath("/infinitelights");
static URI sRendererPath("/renderer");
static URI sIntegratorPath("/renderer/integrator");

URI Registry::getGroupPrefix(RegistryGroup grp)
{
	switch (grp) {
	default:
	case RG_NONE:
		return sNonePath;
	case RG_GLOBAL:
		return sGlobalsPath;
	case RG_MATERIAL:
		return sMaterialsPath;
	case RG_EMISSION:
		return sEmissionsPath;
	case RG_ENTITY:
		return sEntitiesPath;
	case RG_CAMERA:
		return sCamerasPath;
	case RG_INFINITELIGHT:
		return sInfiniteLightsPath;
	case RG_RENDERER:
		return sRendererPath;
	case RG_INTEGRATOR:
		return sIntegratorPath;
	}
}

Registry::Registry()
{
}

void Registry::clear()
{
	mData.clear();
}

void Registry::set(const URI& absUri, const Any& p)
{
	if (!absUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (!absUri.isAbsolute())
		throw std::runtime_error("Absolute URI expected");

	mData[absUri] = p;
}

bool Registry::exists(const URI& absUri) const
{
	if (!absUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (!absUri.isAbsolute())
		throw std::runtime_error("Absolute URI expected");

	return mData.count(absUri) != 0;
}

void Registry::setByGroup(RegistryGroup grp, const URI& relUri, const Any& p)
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (grp != RG_NONE && relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	set(URI::makeAbsolute(relUri, getGroupPrefix(grp)), p);
}

bool Registry::existsByGroup(RegistryGroup grp, const URI& relUri) const
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (grp != RG_NONE && relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	return exists(URI::makeAbsolute(relUri, getGroupPrefix(grp)));
}

void Registry::setForObject(RegistryGroup grp, uint32 id, const URI& relUri, const Any& p)
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (grp != RG_NONE && relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	URI base = getGroupPrefix(grp);
	std::stringstream stream;
	stream << base.path() << "/objects/" << id;
	base.setPath(stream.str());

	const URI absURI = URI::makeAbsolute(relUri, base);

	set(absURI, p);
}

bool Registry::existsForObject(RegistryGroup grp, uint32 id, const URI& relUri, bool useGlobalFallback)
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (grp != RG_NONE && relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	URI base = getGroupPrefix(grp);
	std::stringstream stream;
	stream << base.path() << "/objects/" << id;
	base.setPath(stream.str());

	const URI absURI = URI::makeAbsolute(relUri, base);
	const bool b	 = exists(absURI);
	if (!useGlobalFallback || b)
		return b;
	else
		return exists(URI::makeAbsolute(relUri, sGlobalsPath));
}

std::string Registry::dump() const
{
	// Before output, sort to make it more appealing
	std::vector<std::pair<URI, Any>> elems(mData.begin(), mData.end());
	std::sort(elems.begin(), elems.end(),
			  [](const std::pair<URI, Any>& p1, const std::pair<URI, Any>& p2) {
				  return p1.first.str() < p2.first.str();
			  });

	std::stringstream stream;
	for (const std::pair<URI, Any>& p : elems) {
		std::string c = p.second.cast<std::string>();

		// Simplify for standard types
		std::string typeName;
		if (strcmp(p.second.typeName(), typeid(std::string).name()) == 0)
			typeName = "string";
		else if (strcmp(p.second.typeName(), typeid(std::wstring).name()) == 0)
			typeName = "wstring";
		else if (strcmp(p.second.typeName(), typeid(std::vector<float>).name()) == 0)
			typeName = "vector<float>";
		else
			typeName = boost::core::demangle(p.second.typeName());

		stream << p.first.str() << " [" << typeName << "]"
			   << " = " << c << std::endl;
	}

	return stream.str();
}
} // namespace PR
