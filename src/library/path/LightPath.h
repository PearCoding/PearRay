#pragma once

#include "LightPathToken.h"
#include <vector>

namespace PR {
class PR_LIB LightPath {
public:
	LightPath(size_t expectedSize = 64);
	~LightPath() = default;

	inline void addToken(const LightPathToken& token);
	inline void addToken(LightPathToken&& token);
	inline void popToken();
	inline void popToken(size_t n);
	inline void reset();

	inline size_t containerSize() const;
	inline size_t currentSize() const;
	inline const LightPathToken& token(size_t index) const;

	// Camera -> n*Diffuse -> Light
	static LightPath createCDL(uint32 diffuseCount = 1);
	// Camera -> Background
	static LightPath createCB();
private:
	std::vector<LightPathToken> mTokens;
	size_t mCurrentPos;
};
} // namespace PR

#include "LightPath.inl"