// IWYU pragma: private, include "PR_Config.h"

namespace PR {
/* The intx_t are optional but our system demands them.
 * A system without these types can not use the raytracer
 * due to performance issues anyway.
 */

using int8  = int8_t;
using uint8 = uint8_t;

using int16  = int16_t;
using uint16 = uint16_t;

using int32  = int32_t;
using uint32 = uint32_t;

using int64  = int64_t;
using uint64 = uint64_t;

// Checks
static_assert(sizeof(int8) == 1, "Invalid bytesize configuration");
static_assert(sizeof(uint8) == 1, "Invalid bytesize configuration");
static_assert(sizeof(int16) == 2, "Invalid bytesize configuration");
static_assert(sizeof(uint16) == 2, "Invalid bytesize configuration");
static_assert(sizeof(int32) == 4, "Invalid bytesize configuration");
static_assert(sizeof(uint32) == 4, "Invalid bytesize configuration");
static_assert(sizeof(int64) == 8, "Invalid bytesize configuration");
static_assert(sizeof(uint64) == 8, "Invalid bytesize configuration");

using Point1i = int32;
using Point2i = Eigen::Array2i;
using Point3i = Eigen::Array3i;

using Point1f = float;
using Point2f = Eigen::Array2f;
using Point3f = Eigen::Array3f;

using Size1i = int32;
struct Size2i {
	Size1i Width;
	Size1i Height;

	inline Size2i()
		: Width(0)
		, Height(0)
	{
	}
	inline Size2i(Size1i w, Size1i h)
		: Width(w)
		, Height(h)
	{
	}

	inline int32 area() const { return Width * Height; }
	inline bool isValid() const { return Width > 0 && Height > 0; }

	inline Point2i asArray() const { return Point2i(Width, Height); }
	inline static Size2i fromArray(const Point2i& arr) { return Size2i(arr(0), arr(1)); }
	inline static Size2i fromPoints(const Point2i& start, const Point2i& end) { return Size2i(end(0) - start(0), end(1) - start(1)); }
};

inline bool operator==(const Size2i& left, const Size2i& right)
{
	return left.Width == right.Width && left.Height == right.Height;
}

inline bool operator!=(const Size2i& left, const Size2i& right)
{
	return left.Width != right.Width || left.Height != right.Height;
}

inline Point2i operator+(const Point2i& left, const Size2i& right)
{
	return Point2i(left(0) + right.Width, left(1) + right.Height);
}

inline Point2i operator+(const Size2i& left, const Point2i& right)
{
	return Point2i(left.Width + right(0), left.Height + right(1));
}

inline Point2i operator-(const Point2i& left, const Size2i& right)
{
	return Point2i(left(0) - right.Width, left(1) - right.Height);
}

inline Point2i operator-(const Size2i& left, const Point2i& right)
{
	return Point2i(left.Width - right(0), left.Height - right(1));
}

struct Rect2i {
	Point2i Origin;
	Size2i Size;

	inline Rect2i()
		: Origin(0, 0)
		, Size(1, 1)
	{
	}

	inline Rect2i(const Point2i& origin, const Size2i& size)
		: Origin(origin)
		, Size(size)
	{
	}
};

inline bool operator==(const Rect2i& left, const Rect2i& right)
{
	return (left.Origin == right.Origin).all() && left.Size == right.Size;
}

inline bool operator!=(const Rect2i& left, const Rect2i& right)
{
	return (left.Origin != right.Origin).any() || left.Size != right.Size;
}
} // namespace PR