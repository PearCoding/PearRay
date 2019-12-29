#pragma once

#include "LightPathToken.h"
#include <vector>

namespace PR {
class PR_LIB LightPath {
public:
	LightPath(size_t expectedSize = 64);
	~LightPath() = default;

	LightPath(const LightPath& other) = default;
	LightPath(LightPath&& other)	  = default;

	LightPath& operator=(const LightPath& other) = default;
	LightPath& operator=(LightPath&& other) = default;

	inline const LightPathToken& token(int index) const;

	inline void addToken(const LightPathToken& token);
	inline void addToken(LightPathToken&& token);
	inline void popToken();
	inline void popToken(int n);
	inline void reset();

	inline bool isEmpty() const;
	inline size_t containerSize() const;
	inline size_t currentSize() const;

	// Camera -> n*Diffuse -> Light
	static LightPath createCDL(size_t diffuseCount = 1);
	// Camera -> Background
	static LightPath createCB();

private:
	std::vector<LightPathToken> mTokens;
	size_t mCurrentPos;
};
} // namespace PR

#include "LightPath.inl"