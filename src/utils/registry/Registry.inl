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

	return mData.at(absUri).cast<T>();
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
	if (exists(absURI) || !useGlobalFallback)
		return get<T>(absURI, def);
	else
		return get<T>(URI::makeAbsolute(relUri, getGroupPrefix(RG_GLOBAL)), def);
}
} // namespace PR