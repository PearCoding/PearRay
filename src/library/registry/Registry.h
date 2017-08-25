#pragma once

#include "Parameter.h"
#include "URI.h"

#include <unordered_map>

namespace PR {
class PR_LIB Registry
{
public:
	Registry();

	void clear();

	void set(const URI& absUri, const Parameter& p);
	Parameter get(const URI& absUri, const Parameter& def = Parameter());
	bool exists(const URI& absUri) const;

	void setGlobal(const URI& relUri, const Parameter& p);
	Parameter getGlobal(const URI& relUri, const Parameter& def = Parameter());
	bool existsGlobal(const URI& relUri) const;

	void setMaterial(uint32 matID, const URI& relUri, const Parameter& p);
	Parameter getMaterial(uint32 matID, const URI& relUri, const Parameter& def = Parameter(), bool useGlobalFallback = true);
	bool existsMaterial(uint32 matID, const URI& relUri, bool useGlobalFallback = true);

private:
	std::unordered_map<URI, Parameter, URIHash> mData;
};
}
