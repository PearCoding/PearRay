#pragma once

#include "URI.h"
#include "Any.h"

#include <unordered_map>

namespace PR {
/** @brief Common path prefixes */
enum RegistryGroup {
	RG_NONE,
	RG_GLOBAL,
	RG_MATERIAL,
	RG_EMISSION,
	RG_ENTITY,
	RG_CAMERA,
	RG_INFINITELIGHT,
	RG_RENDERER,
	RG_INTEGRATOR,
};

class PR_LIB_UTILS Registry {
public:
	Registry();

	void clear();

	void set(const URI& absUri, const Any& p);
	bool exists(const URI& absUri) const;
	template<typename T>
	inline T get(const URI& absUri, const T& def) const;

	void setByGroup(RegistryGroup grp, const URI& relUri, const Any& p);
	bool existsByGroup(RegistryGroup grp, const URI& relUri) const;
	template<typename T>
	inline T getByGroup(RegistryGroup grp, const URI& relUri, const T& def) const;

	void setForObject(RegistryGroup grp, uint32 id, const URI& relUri, const Any& p);
	bool existsForObject(RegistryGroup grp, uint32 id, const URI& relUri, bool useGlobalFallback = true);
	template<typename T>
	inline T getForObject(RegistryGroup grp, uint32 id, const URI& relUri, const T& def, bool useGlobalFallback = true) const;

	static URI getGroupPrefix(RegistryGroup);

	std::string dump() const;
private:
	std::unordered_map<URI, Any, URIHash> mData;
};
}

#include "Registry.inl"