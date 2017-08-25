namespace PR {

inline URI::URI()
{
}

inline URI::URI(const std::string& uri)
{
	parse(uri);
}

inline URI& URI::operator=(const std::string& uri)
{
	parse(uri);
	return *this;
}

inline void URI::setFromString(const std::string& uri)
{
	parse(uri);
}

inline std::string URI::str() const
{
	std::stringstream stream;
	if (!protocol().empty())
		stream << protocol() << "://";

	stream << authority();
	stream << path();
	if (!query().empty())
		stream << "?" << query();

	if (!fragment().empty())
		stream << "#" << fragment();

	return stream.str();
}

inline bool URI::isExternal() const
{
	if (host().empty()
		|| host() == "localhost")
		return false;

	return true;
}

inline bool URI::isAbsolute() const
{
	return (!path().empty() && path().at(0) == '/');
}

inline bool URI::isValid() const
{
	return !mPath.empty();
}

inline const std::string& URI::protocol() const
{
	return mProtocol;
}

inline void URI::setProtocol(const std::string& s)
{
	mProtocol = s;
}

inline std::string URI::authority() const
{
	std::stringstream stream;
	stream << host();
	if (!port().empty())
		stream << ":" << port();

	return stream.str();
}

inline const std::string& URI::host() const
{
	return mHost;
}

inline void URI::setHost(const std::string& s)
{
	mHost = s;
}

inline const std::string& URI::port() const
{
	return mPort;
}

inline void URI::setPort(const std::string& s)
{
	mPort = s;
}

inline const std::string& URI::path() const
{
	return mPath;
}

inline void URI::setPath(const std::string& s)
{
	mPath = s;
}

inline const std::string& URI::query() const
{
	return mQuery;
}

inline void URI::setQuery(const std::string& s)
{
	mQuery = s;
}

inline const std::string& URI::fragment() const
{
	return mFragment;
}

inline void URI::setFragment(const std::string& s)
{
	mFragment = s;
}
}