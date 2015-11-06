#pragma once

#include "Config.h"
#include "PearMath.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class Ray
	{
	public:
		Ray(const PM::vec3& pos, const PM::vec3& dir, size_t depth = 0);
		virtual ~Ray();

		void setStartPosition(const PM::vec3& p);
		PM::vec3 startPosition() const;

		void setDirection(const PM::vec3& p);
		PM::vec3 direction() const;

		size_t depth() const;

		void setSpectrum(const Spectrum& s);
		Spectrum spectrum() const;

	private:
		PM::vec3 mStartPosition;
		PM::vec3 mDirection;
		size_t mDepth;// Recursion depth!

		Spectrum mSpectrum;
	};
}