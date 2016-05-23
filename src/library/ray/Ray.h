#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	enum RayFlags
	{
		RF_NeedCollisionNormal = 0x1,
		RF_NeedCollisionUV = 0x2,

		RF_DefaultCollision = RF_NeedCollisionNormal | RF_NeedCollisionUV,

		RF_ShadowRay = 0x10,
	};

	class PR_LIB Ray
	{
	public:
		Ray();
		Ray(const PM::vec3& pos, const PM::vec3& dir, uint32 depth = 0);
		virtual ~Ray();

		inline void setStartPosition(const PM::vec3& p);
		inline PM::vec3 startPosition() const;

		inline void setDirection(const PM::vec3& p);
		inline PM::vec3 direction() const;

		inline void setDepth(uint32 depth);
		inline uint32 depth() const;

		inline uint32 maxDepth() const;
		inline void setMaxDepth(uint32 i);

		inline int flags() const;
		inline void setFlags(int flags);
	private:
		alignas(16) PM::vec3 mStartPosition;
		alignas(16) PM::vec3 mDirection;
		uint32 mDepth;// Recursion depth!
		uint32 mMaxDepth;// If 0 -> renderer->MaxDepth!
		uint8 mFlags;
	};
}

#include "Ray.inl"