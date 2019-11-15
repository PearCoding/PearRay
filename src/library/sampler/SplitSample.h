#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB SplitSample2D {
public:
	inline SplitSample2D(const Vector2f& u,
						 size_t start1, size_t end1,
						 size_t start2, size_t end2)
	{
		PR_ASSERT(start1 < end1, "Expect end to be bigger then start");
		PR_ASSERT(start2 < end2, "Expect end to be bigger then start");

		float k;

		size_t s = end1 - start1;
		mU1		 = std::modf(u(0) * s, &k);
		mI1		 = static_cast<uint32>(k);

		s   = end2 - start2;
		mU2 = std::modf(u(1) * s, &k);
		mI2 = static_cast<uint32>(k);
	}

	inline SplitSample2D(const Vector2f& u,
					size_t start, size_t end)
		: SplitSample2D(u, start, end, start, end)
	{
	}

	~SplitSample2D() = default;

	inline float uniform1() const { return mU1; }
	inline float uniform2() const { return mU2; }
	inline uint32 integral1() const { return mI1; }
	inline uint32 integral2() const { return mI2; }

private:
	float mU1;
	float mU2;
	uint32 mI1;
	uint32 mI2;
};
} // namespace PR
