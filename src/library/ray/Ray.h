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

		RF_NoDirectShading = 0x10,
		RF_NoIndirectShading = 0x11,

		RF_HasTarget = 0x20,

		RF_ShadowRay = /*RF_NoDirectShading |*/ RF_NoIndirectShading | RF_HasTarget,
	};

	class PR_LIB Ray
	{
	public:
		Ray(const PM::vec3& pos, const PM::vec3& dir, uint32 depth = 0);
		virtual ~Ray();

		inline void setStartPosition(const PM::vec3& p);
		inline PM::vec3 startPosition() const;

		inline void setDirection(const PM::vec3& p);
		inline PM::vec3 direction() const;

		inline uint32 depth() const;

		inline void setSpectrum(const Spectrum& s);
		inline Spectrum spectrum() const;

		inline uint32 maxDepth() const;
		inline void setMaxDepth(uint32 i);

		inline int flags() const;
		inline void setFlags(int flags);

		inline void setTarget(const PM::vec3& p);
		inline PM::vec3 target() const;
	private:
		PM::vec3 mStartPosition;
		PM::vec3 mDirection;
		PM::vec3 mTarget;
		uint32 mDepth;// Recursion depth!
		uint32 mMaxDepth;// If 0 -> renderer->MaxDepth!
		uint8 mFlags;

		Spectrum mSpectrum;
	};
}

#include "Ray.inl"