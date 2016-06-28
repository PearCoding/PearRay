#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	constexpr float RayOffsetEpsilon = 0.000001f;
	class PR_LIB Ray
	{
	public:
		Ray();
		Ray(const PM::vec3& pos, const PM::vec3& dir, uint32 depth = 0, float time = 0, uint32 maxDepth = 0);
		virtual ~Ray();

		inline void setStartPosition(const PM::vec3& p);
		inline PM::vec3 startPosition() const;

		inline void setDirection(const PM::vec3& p);
		inline PM::vec3 direction() const;

		inline void setDepth(uint32 depth);
		inline uint32 depth() const;

		inline float time() const;
		inline void setTime(float t);

		inline uint32 maxDepth() const;
		inline void setMaxDepth(uint32 i);

		inline Ray next(const PM::vec3& pos, const PM::vec3& dir) const;
	private:
		alignas(16) PM::vec3 mStartPosition;
		alignas(16) PM::vec3 mDirection;
		uint32 mDepth;// Recursion depth!
		float mTime;
		uint32 mMaxDepth;// If 0 -> renderer->MaxDepth!
	};
}

#include "Ray.inl"