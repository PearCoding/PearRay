#include "Ray.h"

namespace PR
{
	Ray::Ray(const PM::vec3& pos, const PM::vec3& dir, size_t depth) :
		mStartPosition(pos), mDirection(dir), mDepth(depth)
	{
	}

	Ray::~Ray()
	{
	}

	void Ray::setStartPosition(const PM::vec3& p)
	{
		mStartPosition = p;
	}

	PM::vec3 Ray::startPosition() const
	{
		return mStartPosition;
	}

	void Ray::setDirection(const PM::vec3& dir)
	{
		mDirection = dir;
	}

	PM::vec3 Ray::direction() const
	{
		return mDirection;
	}

	size_t Ray::depth() const
	{
		return mDepth;
	}

	void Ray::setSpectrum(const Spectrum& s)
	{
		mSpectrum = s;
	}

	Spectrum Ray::spectrum() const
	{
		return mSpectrum;
	}
}