#pragma once

#include "LightPath.h"
#include "shader/ShadingPoint.h"

namespace PR {
enum LightPathBufferEntryFlags : uint32 {
	LPBEF_SCATTER_REFLECTION = 0x1,
	LPBEF_EVENT_DIFFUSE		 = 0x2
};

struct PR_LIB_INLINE LightPathBufferEntry {
	uint32 Flags;
	uint32 LabelIndex;

	inline LightPathToken tokenize() const
	{
		return LightPathToken(
			(Flags & LPBEF_SCATTER_REFLECTION) ? ST_REFLECTION : ST_REFRACTION,
			(Flags & LPBEF_EVENT_DIFFUSE) ? SE_DIFFUSE : SE_SPECULAR,
			LabelIndex);
	}
};

class PR_LIB LightPathBuffer {
public:
	LightPathBuffer(size_t maxRays, size_t maxLength);
	~LightPathBuffer() = default;

	void reset();
	bool add(size_t ray_id, const LightPathBufferEntry& entry);
	size_t length(size_t ray_id) const;
	LightPath getPath(size_t ray_id, size_t maxLength = std::numeric_limits<size_t>::max()) const;
	LightPathBufferEntry getEntry(size_t ray_id, size_t entry) const;

private:
	size_t mMaxRays;
	size_t mMaxLength;
	size_t mRowStride;
	std::vector<uint8> mData;
};
} // namespace PR