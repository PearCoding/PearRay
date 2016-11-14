#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	enum RayFlags
	{
		RF_FromLight = 0x1
	};

	constexpr float RayOffsetEpsilon = 0.000001f;
	class PR_LIB Ray
	{
	public:
		Ray();
		Ray(const PM::vec3& pixel, const PM::vec3& pos, const PM::vec3& dir, uint32 depth = 0, float time = 0, uint16 flags = 0, uint32 maxDepth = 0);
		virtual ~Ray();

		inline void setStartPosition(const PM::vec3& p);
		inline PM::vec3 startPosition() const;

		inline void setDirection(const PM::vec3& p);
		inline PM::vec3 direction() const;

		inline void setPixel(const PM::vec2& p);
		inline PM::vec2 pixel() const;

		inline void setDepth(uint32 depth);
		inline uint32 depth() const;

		inline float time() const;
		inline void setTime(float t);

		inline void setFlags(uint16 flags);
		inline uint16 flags() const;

		inline uint32 maxDepth() const;
		inline void setMaxDepth(uint32 i);

		// Used by Triangle.h
#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		inline uint32 maxDirectionIndex() const;
#endif

		inline Ray next(const PM::vec3& pos, const PM::vec3& dir) const;
		static inline Ray safe(const PM::vec3& pixel, const PM::vec3& pos, const PM::vec3& dir,
			uint32 depth = 0, float time = 0, uint16 flags = 0, uint32 maxDepth = 0);
	private:
		alignas(16) PM::vec3 mStartPosition;
		alignas(16) PM::vec3 mDirection;
		alignas(16) PM::vec2 mPixel;
		uint32 mDepth;// Recursion depth!
		float mTime;
		uint16 mFlags;
		uint32 mMaxDepth;// If 0 -> renderer->MaxDepth!

#if PR_TRIANGLE_INTERSECTION_TECHNIQUE == 1
		inline void calcMaxDirectionElement();
		uint32 mMaxDirectionIndex;
#endif
	};
}

#include "Ray.inl"