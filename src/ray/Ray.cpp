#include "Ray.h"

namespace PR
{
	Ray::Ray(const PM::vec3& pos, const PM::vec3& dir) :
		mStartPosition(pos), mDirection(dir)
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

	void Ray::setSpectrum(const Spectrum& s)
	{
		mSpectrum = s;
	}

	Spectrum Ray::spectrum() const
	{
		return mSpectrum;
	}
}