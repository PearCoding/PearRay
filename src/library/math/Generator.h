#pragma once

#include "PR_Config.h"

#include <array>
#include <queue>
#include <unordered_set>

#include <boost/functional/hash/hash.hpp>

/* This classes here really need C++XX Coroutine feature */
namespace PR {
// Breadth-First-Search
// TODO: Solution without dynamic containers?
template <int D>
class MinRadiusGenerator {
public:
	constexpr static uint32 Dimension = D;
	typedef std::array<int32, Dimension> point_type;

	explicit MinRadiusGenerator(uint32 radius)
		: mRadius(radius)
	{
		mVisited.reserve(static_cast<size_t>(std::pow(2 * radius + 1, Dimension)));

		point_type p;
		for (uint32 i = 0; i < Dimension; ++i)
			p[i] = 0;

		mQueue.push(p);
		mVisited.insert(p);
	}

	operator bool() const
	{
		return hasNext();
	}

	bool hasNext() const
	{
		return !mQueue.empty();
	}

	point_type next()
	{
		if (!hasNext())
			return point_type();

		const point_type p = mQueue.front();
		mQueue.pop();

		point_type c = p;
		for (uint32 i = 0; i < Dimension; ++i) {
			if (p[i] < (int32)mRadius) {
				c[i] = p[i] + 1;

				if (!mVisited.count(c)) // Not concurrent!
				{
					mQueue.push(c);
					mVisited.insert(c);
				}
			}

			if (p[i] > -(int32)mRadius) {
				c[i] = p[i] - 1;

				if (!mVisited.count(c)) // Not concurrent!
				{
					mQueue.push(c);
					mVisited.insert(c);
				}
			}

			c[i] = p[i];
		}

		return p;
	}

private:
	class PointHash {
	public:
		size_t operator()(const point_type& s) const
		{
			return boost::hash_range(s.begin(), s.end());
		}
	};

	const uint32 mRadius;
	std::queue<point_type> mQueue;
	std::unordered_set<point_type, PointHash> mVisited;
};
} // namespace PR
