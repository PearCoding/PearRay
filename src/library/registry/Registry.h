#pragma once

#include "Parameter.h"
#include "URI.h"

#include <unordered_map>

namespace PR {
enum RegistryGroup {
	RG_Global,
	RG_Material,
	RG_Entity,
	RG_Integrator,
};

class PR_LIB Registry {
public:
	Registry();

	void clear();

	void set(const URI& absUri, const Parameter& p);
	Parameter get(const URI& absUri, const Parameter& def = Parameter());
	bool exists(const URI& absUri) const;

	void setByGroup(RegistryGroup grp, const URI& relUri, const Parameter& p);
	Parameter getByGroup(RegistryGroup grp, const URI& relUri, const Parameter& def = Parameter());
	bool existsByGroup(RegistryGroup grp, const URI& relUri) const;

	void setForObject(RegistryGroup grp, uint32 id, const URI& relUri, const Parameter& p);
	Parameter getForObject(RegistryGroup grp, uint32 id, const URI& relUri, const Parameter& def = Parameter(), bool useGlobalFallback = true);
	bool existsForObject(RegistryGroup grp, uint32 id, const URI& relUri, bool useGlobalFallback = true);

	static URI getGroupPrefix(RegistryGroup);

	std::string dump() const;
private:
	std::unordered_map<URI, Parameter, URIHash> mData;
};
}
