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
	constexpr static int Dimension = D;
	typedef std::array<int, Dimension> point_type;

	explicit MinRadiusGenerator(int radius)
		: mRadius(radius)
	{
		mVisited.reserve(std::pow(2 * radius + 1, Dimension));

		point_type p;
		for (int i = 0; i < Dimension; ++i)
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
		for (int i = 0; i < Dimension; ++i) {
			if (p[i] < mRadius) {
				c[i] = p[i] + 1;

				if (!mVisited.count(c)) // Not concurrent!
				{
					mQueue.push(c);
					mVisited.insert(c);
				}
			}

			if (p[i] > -mRadius) {
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

	const int mRadius;
	std::queue<point_type> mQueue;
	std::unordered_set<point_type, PointHash> mVisited;
};
} // namespace PR
