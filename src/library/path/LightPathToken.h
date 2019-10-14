#pragma once

#include "PR_Config.h"

namespace PR {
enum ScatteringType {
	ST_CAMERA = 0,
	ST_EMISSIVE,
	ST_REFRACTION,
	ST_REFLECTION,
	ST_BACKGROUND,
	_ST_COUNT
};

enum ScatteringEvent {
	SE_DIFFUSE = 0,
	SE_SPECULAR,
	SE_NONE,
	_SE_COUNT
};

struct PR_LIB_INLINE LightPathToken {
	ScatteringType Type;
	ScatteringEvent Event;
	size_t LabelIndex;

	inline LightPathToken(ScatteringType type, ScatteringEvent event, size_t labelIndex = 0)
		: Type(type)
		, Event(event)
		, LabelIndex(labelIndex)
	{
	}
};
} // namespace PR