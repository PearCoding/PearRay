#pragma once

#include "Config.h"
#include "PearMath.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB Ray
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

		size_t maxDepth() const;
		void setMaxDepth(size_t i);
	private:
		PM::vec3 mStartPosition;
		PM::vec3 mDirection;
		size_t mDepth;// Recursion depth!
		size_t mMaxDepth;// If 0 -> renderer->MaxDepth!

		Spectrum mSpectrum;
	};
}