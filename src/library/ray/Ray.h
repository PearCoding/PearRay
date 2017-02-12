#pragma once

#include "PR_Config.h"
#include "PearMath.h"

namespace PR
{
	enum RayFlags
	{
		RF_Light 	= 0x1,
		RF_Debug 	= 0x2
	};

	constexpr float RayOffsetEpsilon = 0.000001f;
	class PR_LIB Ray
	{
	public:
		Ray();
		Ray(uint32 px, uint32 py, const PM::vec3& pos, const PM::vec3& dir, uint32 depth = 0, float time = 0, uint16 flags = 0);
		virtual ~Ray();

		inline void setStartPosition(const PM::vec3& p);
		inline PM::vec3 startPosition() const;

		inline void setDirection(const PM::vec3& p);
		inline PM::vec3 direction() const;

		inline void setPixelX(uint32 px);
		inline uint32 pixelX() const;

		inline void setPixelY(uint32 py);
		inline uint32 pixelY() const;

		inline void setDepth(uint32 depth);
		inline uint32 depth() const;

		inline float time() const;
		inline void setTime(float t);

		inline void setFlags(uint16 flags);
		inline uint16 flags() const;

		// Used by Triangle.h
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		inline uint32 maxDirectionIndex() const;
#endif

		inline Ray next(const PM::vec3& pos, const PM::vec3& dir) const;
		static inline Ray safe(uint32 px, uint32 py, const PM::vec3& pos, const PM::vec3& dir,
			uint32 depth = 0, float time = 0, uint16 flags = 0);
	private:
		alignas(16) PM::vec3 mStartPosition;
		alignas(16) PM::vec3 mDirection;
		uint32 mPixelX; uint32 mPixelY;
		uint32 mDepth;// Recursion depth!
		float mTime;
		uint16 mFlags;

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		inline void calcMaxDirectionElement();
		uint32 mMaxDirectionIndex;
#endif
	};
}

#include "Ray.inl"
