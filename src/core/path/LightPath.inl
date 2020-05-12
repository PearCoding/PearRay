// IWYU pragma: private, include "path/LightPath.h"
namespace PR {
inline void LightPath::addToken(const LightPathToken& token)
{
	if (mCurrentPos >= mTokens.size()) {
		mTokens.emplace_back(token);
		mCurrentPos = mTokens.size();
	} else {
		mTokens[mCurrentPos] = token;
		++mCurrentPos;
	}
}

inline void LightPath::addToken(LightPathToken&& token)
{
	if (mCurrentPos >= mTokens.size()) {
		mTokens.emplace_back(std::move(token));
		mCurrentPos = mTokens.size();
	} else {
		mTokens[mCurrentPos] = std::move(token);
		++mCurrentPos;
	}
}

inline void LightPath::popToken()
{
	PR_ASSERT(mCurrentPos > 0, "Nothing to remove!");
	--mCurrentPos;
}

inline void LightPath::popToken(int n)
{
	for (int i = 0; i < n; ++i)
		popToken();
}

inline void LightPath::reset()
{
	mCurrentPos = 0;
}

inline bool LightPath::isEmpty() const
{
	return mCurrentPos == 0;
}

inline size_t LightPath::containerSize() const
{
	return mTokens.size();
}

inline size_t LightPath::currentSize() const
{
	return mCurrentPos;
}

const LightPathToken& LightPath::token(int index) const
{
	return mTokens[index];
}

inline size_t LightPath::packedSizeRequirement() const
{
	return (currentSize() + 1) * sizeof(uint32);
}

inline void LightPath::toPacked(uint8* buffer, size_t size) const
{
	PR_UNUSED(size);// Remove warning when asserts are not used
	PR_ASSERT(size >= packedSizeRequirement(), "Minimum buffer size not satisfied");

	uint32* b = reinterpret_cast<uint32*>(buffer);
	b[0]	  = currentSize();
	for (size_t i = 0; i < currentSize(); ++i)
		b[i + 1] = mTokens[i].toPacked();
}

inline void LightPath::addFromPacked(const uint8* buffer, size_t size)
{
	PR_UNUSED(size);
	PR_ASSERT(size >= sizeof(uint32), "At least size information is required");
	const uint32* b = reinterpret_cast<const uint32*>(buffer);
	uint32 elems	= b[0];
	PR_ASSERT(size >= (elems + 1) * sizeof(uint32), "Buffer is not well sized");
	for (uint32 i = 0; i < elems; ++i)
		addToken(LightPathToken::fromPacked(b[i + 1]));
}
} // namespace PR