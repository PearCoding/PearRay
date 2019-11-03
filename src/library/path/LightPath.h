#pragma once

#include "LightPathToken.h"
#include <vector>

namespace PR {
class PR_LIB LightPath {
public:
	LightPath(size_t expectedSize = 64);
	~LightPath() = default;

	void addToken(const LightPathToken& token);

	inline size_t currentSize() const { return mTokens.size(); }
	const LightPathToken& token(size_t index) const { return mTokens[index]; }

	void concat(const LightPath& other);
	inline void concat(const LightPathToken& token) { addToken(token); }

	inline LightPath concated(const LightPath& other) const
	{
		LightPath tmp = *this;
		tmp.concat(other);
		return tmp;
	}

	inline LightPath concated(const LightPathToken& token) const
	{
		LightPath tmp = *this;
		tmp.concat(token);
		return tmp;
	}

	static LightPath createCDL(uint32 diffuseCount = 1);

private:
	std::vector<LightPathToken> mTokens;
};
} // namespace PR