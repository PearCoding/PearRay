#pragma once

#include "PR_Config.h"
#include <sstream>

namespace PR {
class PR_LIB URI {
public:
	inline URI();
	inline explicit URI(const std::string& uri);

	inline URI& operator=(const std::string& uri);

	inline void setFromString(const std::string& s);
	inline std::string str() const;

	inline bool isExternal() const;
	inline bool isAbsolute() const;
	inline bool isValid() const;

	inline const std::string& protocol() const;
	inline void setProtocol(const std::string& s);

	inline std::string authority() const;

	inline const std::string& host() const;
	inline void setHost(const std::string& s);

	inline const std::string& port() const;
	inline void setPort(const std::string& s);

	inline const std::string& path() const;
	inline void setPath(const std::string& s);

	inline const std::string& query() const;
	inline void setQuery(const std::string& s);

	inline const std::string& fragment() const;
	inline void setFragment(const std::string& s);

	static URI makeAbsolute(const URI& p, const URI& base);

private:
	void parse(const std::string& s);

	std::string mProtocol;
	std::string mHost;
	std::string mPort;
	std::string mPath;
	std::string mQuery;
	std::string mFragment;
};

inline bool operator==(const URI& a, const URI& b) { return a.str() == b.str(); }

struct URIHash {
	inline size_t operator()(const URI& x) const { return std::hash<std::string>()(x.str()); }
};
} // namespace PR

#include "URI.inl"
