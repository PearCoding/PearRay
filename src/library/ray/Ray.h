#pragma once

#include "Config.h"
#include "PearMath.h"
#include "spectral/Spectrum.h"

namespace PR
{
	enum RayFlags
	{
		RF_NeedCollisionNormal = 0x1,
		RF_NeedCollisionUV = 0x2,

		RF_DefaultCollision = RF_NeedCollisionNormal | RF_NeedCollisionUV,
	};

	class PR_LIB Ray
	{
	public:
		Ray(const PM::vec3& pos, const PM::vec3& dir, size_t depth = 0);
		virtual ~Ray();

		inline void setStartPosition(const PM::vec3& p);
		inline PM::vec3 startPosition() const;

		inline void setDirection(const PM::vec3& p);
		inline PM::vec3 direction() const;

		inline size_t depth() const;

		inline void setSpectrum(const Spectrum& s);
		inline Spectrum spectrum() const;

		inline size_t maxDepth() const;
		inline void setMaxDepth(size_t i);

		inline int flags() const;
		inline void setFlags(int flags);

	private:
		PM::vec3 mStartPosition;
		PM::vec3 mDirection;
		size_t mDepth;// Recursion depth!
		size_t mMaxDepth;// If 0 -> renderer->MaxDepth!
		int mFlags;

		Spectrum mSpectrum;
	};
}

#include "Ray.inl"