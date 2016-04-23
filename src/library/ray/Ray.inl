#pragma once

namespace PR
{
	inline void Ray::setStartPosition(const PM::vec3& p)
	{
		mStartPosition = p;
	}

	inline PM::vec3 Ray::startPosition() const
	{
		return mStartPosition;
	}

	inline void Ray::setDirection(const PM::vec3& dir)
	{
		mDirection = dir;
	}

	inline PM::vec3 Ray::direction() const
	{
		return mDirection;
	}

	inline size_t Ray::depth() const
	{
		return mDepth;
	}

	inline void Ray::setSpectrum(const Spectrum& s)
	{
		mSpectrum = s;
	}

	inline Spectrum Ray::spectrum() const
	{
		return mSpectrum;
	}

	inline size_t Ray::maxDepth() const
	{
		return mMaxDepth;
	}

	inline void Ray::setMaxDepth(size_t i)
	{
		mMaxDepth = i;
	}

	inline int Ray::flags() const
	{
		return mFlags;
	}

	inline void Ray::setFlags(int flags)
	{
		mFlags = flags;
	}
}