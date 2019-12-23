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

inline void LightPath::popToken(size_t n)
{
	for (size_t i = 0; i < n; ++i)
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

const LightPathToken& LightPath::token(size_t index) const
{
	return mTokens[index];
}

} // namespace PR