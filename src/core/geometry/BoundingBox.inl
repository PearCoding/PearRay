// IWYU pragma: private, include "geometry/BoundingBox.h"

namespace PR {
inline BoundingBox::BoundingBox()
	: mUpperBound(-PR_INF, -PR_INF, -PR_INF)
	, mLowerBound(PR_INF, PR_INF, PR_INF)
{
}

inline BoundingBox::BoundingBox(const Vector3f& upperbound, const Vector3f& lowerbound)
	: mUpperBound(upperbound.cwiseMax(lowerbound))
	, mLowerBound(lowerbound.cwiseMin(upperbound))
{
}

inline BoundingBox::BoundingBox(float width, float height, float depth)
	: mUpperBound(width / 2, height / 2, depth / 2)
	, mLowerBound(-width / 2, -height / 2, -depth / 2)
{
	PR_ASSERT(width >= 0, "width has to be positive");
	PR_ASSERT(height >= 0, "height has to be positive");
	PR_ASSERT(depth >= 0, "depth has to be positive");
}

inline Vector3f BoundingBox::upperBound() const { return mUpperBound; }
inline Vector3f& BoundingBox::upperBound() { return mUpperBound; }
inline void BoundingBox::setUpperBound(const Vector3f& bound) { mUpperBound = bound; }

inline Vector3f BoundingBox::lowerBound() const { return mLowerBound; }
inline Vector3f& BoundingBox::lowerBound() { return mLowerBound; }
inline void BoundingBox::setLowerBound(const Vector3f& bound) { mLowerBound = bound; }

inline Vector3f BoundingBox::center() const { return mLowerBound + (mUpperBound - mLowerBound) * 0.5f; }

inline float BoundingBox::width() const { return edge(0); }
inline float BoundingBox::height() const { return edge(1); }
inline float BoundingBox::depth() const { return edge(2); }
inline float BoundingBox::edge(int dim) const { return mUpperBound(dim) - mLowerBound(dim); }

inline float BoundingBox::longestEdge() const { return std::max(width(), std::max(height(), depth())); }
inline float BoundingBox::shortestEdge() const { return std::min(width(), std::min(height(), depth())); }

inline float BoundingBox::diameter() const { return (mUpperBound - mLowerBound).norm(); }

inline float BoundingBox::volume() const { return width() * height() * depth(); }
inline float BoundingBox::surfaceArea() const { return 2 * (width() * height() + width() * depth() + height() * depth()); }

inline float BoundingBox::faceArea(int dim) const
{
	switch (dim) {
	case 0:
		return edge(1) * edge(2);
	case 1:
		return edge(0) * edge(2);
	case 2:
		return edge(0) * edge(1);
	default:
		return -1;
	}
}

inline bool BoundingBox::isValid() const { return (mLowerBound.array() <= mUpperBound.array()).all(); }

inline bool BoundingBox::isPlanar(float eps) const
{
	return width() <= eps || height() <= eps || depth() <= eps;
}

inline bool BoundingBox::contains(const Vector3f& point) const
{
	return (mUpperBound.array() >= point.array()).all() && (mLowerBound.array() <= point.array()).all();
}

inline void BoundingBox::clipBy(const BoundingBox& other)
{
	mLowerBound = mLowerBound.cwiseMax(other.mLowerBound).cwiseMin(other.mUpperBound);
	mUpperBound = mUpperBound.cwiseMax(other.mLowerBound).cwiseMin(other.mUpperBound);
}

inline void BoundingBox::expand(float amount)
{
	mUpperBound += Vector3f(amount, amount, amount);
	mLowerBound -= Vector3f(amount, amount, amount);
}

inline void BoundingBox::combine(const Vector3f& point)
{
	mUpperBound = mUpperBound.cwiseMax(point);
	mLowerBound = mLowerBound.cwiseMin(point);
}

inline void BoundingBox::combine(const BoundingBox& other)
{
	combine(other.lowerBound());
	combine(other.upperBound());
}

inline void BoundingBox::shift(const Vector3f& point)
{
	mUpperBound += point;
	mLowerBound += point;
}

inline BoundingBox BoundingBox::expanded(float amount) const
{
	BoundingBox tmp = *this;
	tmp.expand(amount);
	return tmp;
}

inline BoundingBox BoundingBox::combined(const Vector3f& point) const
{
	BoundingBox tmp = *this;
	tmp.combine(point);
	return tmp;
}

inline BoundingBox BoundingBox::combined(const BoundingBox& other) const
{
	BoundingBox tmp = *this;
	tmp.combine(other);
	return tmp;
}

inline BoundingBox BoundingBox::shifted(const Vector3f& point) const
{
	BoundingBox tmp = *this;
	tmp.shift(point);
	return tmp;
}

inline void BoundingBox::transform(const Eigen::Affine3f& t)
{
	const Eigen::Matrix3f M = t.linear();

	const Vector3f x1 = M.col(0) * mLowerBound(0);
	const Vector3f x2 = M.col(0) * mUpperBound(0);

	const Vector3f y1 = M.col(1) * mLowerBound(1);
	const Vector3f y2 = M.col(1) * mUpperBound(1);

	const Vector3f z1 = M.col(2) * mLowerBound(2);
	const Vector3f z2 = M.col(2) * mUpperBound(2);

	mLowerBound = x1.cwiseMin(x2) + y1.cwiseMin(y2) + z1.cwiseMin(z2);
	mUpperBound = x1.cwiseMax(x2) + y1.cwiseMax(y2) + z1.cwiseMax(z2);

	shift(t.translation());
}

inline BoundingBox BoundingBox::transformed(const Eigen::Affine3f& t) const
{
	BoundingBox tmp = *this;
	tmp.transform(t);
	return tmp;
}

} // namespace PR
