namespace PR {

template <typename T>
inline T Registry::get(const URI& absUri, const T& def) const
{
	if (!absUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (!absUri.isAbsolute())
		throw std::runtime_error("Absolute URI expected");

	if (!exists(absUri))
		return def;

	try {
		return mData.at(absUri).cast<T>();
	} catch(const BadCast&) {
		return def;
	}
}

template <typename T>
inline bool Registry::isType(const URI& absUri) const
{
	if (!exists(absUri))
		return false;

	try {
		mData.at(absUri).cast<T>();
		return true;
	} catch(const BadCast&) {
		return false;
	}
}

template <typename T>
inline T Registry::getByGroup(RegistryGroup grp, const URI& relUri, const T& def) const
{
	if (!relUri.isValid())
		throw std::runtime_error("Invalid URI given");

	if (relUri.isAbsolute())
		throw std::runtime_error("Relative URI expected");

	return get<T>(URI::makeAbsolute(relUri, getGroupPrefix(grp)), def);
}

template <typename T>
inline bool Registry::isTypeByGroup(RegistryGroup grp, const URI& relUri) const
{
	return isType<T>(URI::makeAbsolute(relUri, getGroupPrefix(grp)));
}

template <typename T>
inline T Registry::getForObject(RegistryGroup grp, uint32 id, const URI& relUri,
					  const T& def, bool useGlobalFallback) const
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
	if (!useGlobalFallback || isType<T>(absURI))
		return get<T>(absURI, def);
	else
		return get<T>(URI::makeAbsolute(relUri, getGroupPrefix(RG_GLOBAL)), def);
}

template <typename T>
inline bool Registry::isTypeForObject(RegistryGroup grp, uint32 id, const URI& relUri, bool useGlobalFallback)
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
	if (!isType<T>(absURI)) {
		if (useGlobalFallback)
			return isType<T>(URI::makeAbsolute(relUri, getGroupPrefix(RG_GLOBAL)));
		else
			return false;
	} else {
		return true;
	}
}
}