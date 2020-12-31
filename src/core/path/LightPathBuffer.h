#pragma once

#include "LightPath.h"
#include "trace/IntersectionPoint.h"

namespace PR {
enum class LightPathBufferEntryFlag : uint32 {
	ScatterReflection = 0x1,
	EventDiffuse	  = 0x2
};
PR_MAKE_FLAGS(LightPathBufferEntryFlag, LightPathBufferEntryFlags)

struct PR_LIB_CORE LightPathBufferEntry {
	LightPathBufferEntryFlags Flags;
	uint32 LabelIndex;

	inline LightPathToken tokenize() const
	{
		return LightPathToken(
			(Flags & LightPathBufferEntryFlag::ScatterReflection) ? ScatteringType::Reflection : ScatteringType::Refraction,
			(Flags & LightPathBufferEntryFlag::EventDiffuse) ? ScatteringEvent::Diffuse : ScatteringEvent::Specular,
			LabelIndex);
	}
};

class PR_LIB_CORE LightPathBuffer {
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