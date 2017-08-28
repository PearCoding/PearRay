#include "Registry.h"

namespace PR {
static URI sGlobalsPath("/Globals");
static URI sMaterialsPath("/Materials");
static URI sEntitiesPath("/Entities");
static URI sIntegratorsPath("/Integrators");

URI Registry::getGroupPrefix(RegistryGroup grp)
{
	switch (grp) {
	default:
	case RG_Global:
		return sGlobalsPath;
	case RG_Material:
		return sMaterialsPath;
	case RG_Entity:
		return sEntitiesPath;
	case RG_Integrator:
		return sIntegratorsPath;
	}
}

Registry::Registry()
{
}

void Registry::clear()
{
	mData.clear();
}

void Registry::set(const URI& absUri, const Parameter& p)
{
	if (!absUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (!absUri.isAbsolute())
		throw std::runtime_error("Absolute URI expected");

	mData[absUri] = p;
}

Parameter Registry::get(const URI& absUri, const Parameter& def)
{
	if (!absUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (!absUri.isAbsolute())
		throw std::runtime_error("Absolute URI expected");

	if (mData.count(absUri) != 0)
		return mData[absUri];
	else
		return def;
}

bool Registry::exists(const URI& absUri) const
{
	if (!absUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (!absUri.isAbsolute())
		throw std::runtime_error("Absolute URI expected");

	return mData.count(absUri) != 0;
}

void Registry::setByGroup(RegistryGroup grp, const URI& relUri, const Parameter& p)
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	set(URI::makeAbsolute(relUri, getGroupPrefix(grp)), p);
}

Parameter Registry::getByGroup(RegistryGroup grp, const URI& relUri, const Parameter& def)
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	return get(URI::makeAbsolute(relUri, getGroupPrefix(grp)), def);
}

bool Registry::existsByGroup(RegistryGroup grp, const URI& relUri) const
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	return exists(URI::makeAbsolute(relUri, getGroupPrefix(grp)));
}

void Registry::setForObject(RegistryGroup grp, uint32 id, const URI& relUri, const Parameter& p)
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	URI base = getGroupPrefix(grp);
	std::stringstream stream;
	stream << base.path() << "/objects/" << id;
	base.setPath(stream.str());

	const URI absURI = URI::makeAbsolute(relUri, base);

	set(absURI, p);
}

Parameter Registry::getForObject(RegistryGroup grp, uint32 id, const URI& relUri, const Parameter& def, bool useGlobalFallback)
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	URI base = getGroupPrefix(grp);
	std::stringstream stream;
	stream << base.path() << "/objects/" << id;
	base.setPath(stream.str());

	const URI absURI = URI::makeAbsolute(relUri, base);
	if (!useGlobalFallback || exists(absURI))
		return get(absURI, def);
	else
		return get(URI::makeAbsolute(relUri, sGlobalsPath), def);
}

bool Registry::existsForObject(RegistryGroup grp, uint32 id, const URI& relUri, bool useGlobalFallback)
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (relUri.isAbsolute())
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
	std::stringstream stream;
	for (const std::pair<URI, Parameter>& p : mData) {
		stream << p.first.str() << " [" << p.second.typeString() << "]" << std::endl;
	}

	return stream.str();
}
}
