#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_CORE SplitSample1D {
public:
	inline SplitSample1D(float u,
						 size_t start, size_t end)
	{
		PR_ASSERT(start < end, "Expect end to be bigger then start");

		float k;
		size_t s = end - start;
		mU		 = std::modf(u * s, &k);
		mI		 = std::min<uint32>(static_cast<uint32>(k), end - 1);
	}

	~SplitSample1D() = default;

	inline float uniform() const { return mU; }
	inline uint32 integral() const { return mI; }

private:
	float mU;
	uint32 mI;
};

class PR_LIB_CORE SplitSample2D {
public:
	inline SplitSample2D(const Vector2f& u,
						 size_t start1, size_t end1,
						 size_t start2, size_t end2)
		: mS1(u(0), start1, end1)
		, mS2(u(1), start2, end2)
	{
	}

	inline SplitSample2D(const Vector2f& u,
						 size_t start, size_t end)
		: SplitSample2D(u, start, end, start, end)
	{
	}

	~SplitSample2D() = default;

	inline float uniform1() const { return mS1.uniform(); }
	inline float uniform2() const { return mS2.uniform(); }
	inline uint32 integral1() const { return mS1.integral(); }
	inline uint32 integral2() const { return mS2.integral(); }

private:
	SplitSample1D mS1;
	SplitSample1D mS2;
};
} // namespace PR
