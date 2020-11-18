#pragma once

#include "LightPathToken.h"
#include <vector>

namespace PR {
class PR_LIB_CORE LightPath {
public:
	LightPath(size_t expectedSize = 8);
	~LightPath() = default;

	LightPath(const LightPath& other) = default;
	LightPath(LightPath&& other)	  = default;

	LightPath& operator=(const LightPath& other) = default;
	LightPath& operator=(LightPath&& other) = default;

	/// Access token at given index. No bounds check apply
	inline LightPathToken token(int index) const;

	/// Append a token to the end
	inline void addToken(const LightPathToken& token);
	/// Append a token to the end. This version uses move semantics
	inline void addToken(LightPathToken&& token);
	/// Pop one token. Will assert if no token is available
	inline void popToken();
	/// Pop n tokens. Will assert if no n tokens are available
	inline void popToken(int n);
	/// Pop until i tokens are left, or if already less then i are left, do nothing
	inline void popTokenUntil(int i);
	/// Removes all available tokens
	inline void reset();

	/// Returns true if no tokens are stored
	inline bool isEmpty() const;
	/// Returns full container size, which may be larger then the current size
	inline size_t containerSize() const;
	/// Returns amount of tokens stored
	inline size_t currentSize() const;

	/// Create a 'Camera -> n*Diffuse -> Light' path
	static LightPath createCDL(size_t diffuseCount = 1);
	/// Creates a 'Camera -> Background' path
	static LightPath createCB();

	/// Converts current path to packed representation. Size of the buffer should be atleast as big as the return value of packedSizeRequirement()
	inline void toPacked(uint8* buffer, size_t size) const;
	/// Append tokens from a packed representation
	inline void addFromPacked(const uint8* buffer, size_t size);

	/// Returns size in bytes required to store the path in packed representation
	inline size_t packedSizeRequirement() const;
	inline static size_t packedSizeRequirement(size_t length);

private:
	std::vector<LightPathToken> mTokens;
	size_t mCurrentPos;
};
} // namespace PR

#include "LightPath.inl"