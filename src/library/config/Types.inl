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

using Size1 = uint32;
struct Size2 {
	Size1 Width;
	Size1 Height;

	inline Size2()
		: Width(0)
		, Height(0)
	{
	}
	inline Size2(Size1 w, Size1 h)
		: Width(w)
		, Height(h)
	{
	}

	inline uint32 area() const { return Width * Height; }
	inline bool isValid() const { return area() > 0; }
};
}