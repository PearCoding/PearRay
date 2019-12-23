#pragma once

#include "material/MaterialType.h"

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

struct PR_LIB LightPathToken {
	ScatteringType Type;
	ScatteringEvent Event;
	size_t LabelIndex;

	inline LightPathToken(ScatteringType type = ST_CAMERA, ScatteringEvent event = SE_NONE, size_t labelIndex = 0)
		: Type(type)
		, Event(event)
		, LabelIndex(labelIndex)
	{
	}

	inline LightPathToken(MaterialScatteringType type, size_t labelIndex = 0)
		: LabelIndex(labelIndex)
	{
		switch (type) {
		case MST_DiffuseReflection:
			Type  = ST_REFLECTION;
			Event = SE_DIFFUSE;
			break;
		case MST_DiffuseTransmission:
			Type  = ST_REFRACTION;
			Event = SE_DIFFUSE;
			break;
		case MST_SpecularReflection:
			Type  = ST_REFLECTION;
			Event = SE_SPECULAR;
			break;
		case MST_SpecularTransmission:
			Type  = ST_REFRACTION;
			Event = SE_SPECULAR;
			break;
		}
	}

	LightPathToken(const LightPathToken& other) = default;
	LightPathToken(LightPathToken&& other)		= default;

	LightPathToken& operator=(const LightPathToken& other) = default;
	LightPathToken& operator=(LightPathToken&& other) = default;

	static inline LightPathToken Camera() { return LightPathToken(ST_CAMERA, SE_NONE); }
	static inline LightPathToken Background() { return LightPathToken(ST_BACKGROUND, SE_NONE); }
	static inline LightPathToken Emissive() { return LightPathToken(ST_EMISSIVE, SE_NONE); }
};
} // namespace PR