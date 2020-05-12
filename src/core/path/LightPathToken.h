#pragma once

#include "material/MaterialType.h"

namespace PR {
enum ScatteringType : uint8 {
	ST_CAMERA = 0,
	ST_EMISSIVE,
	ST_REFRACTION,
	ST_REFLECTION,
	ST_BACKGROUND,
	_ST_COUNT
};

enum ScatteringEvent : uint8 {
	SE_DIFFUSE = 0,
	SE_SPECULAR,
	SE_NONE,
	_SE_COUNT
};

struct PR_LIB_CORE LightPathToken {
private:
	union _LightPathTokenPacked {
		struct {
			uint8 T;
			uint8 E;
			uint16 L;
		} _S;
		uint32 _I;
	};

public:
	ScatteringType Type;
	ScatteringEvent Event;
	uint16 LabelIndex;

	inline LightPathToken(ScatteringType type = ST_CAMERA, ScatteringEvent event = SE_NONE, uint16 labelIndex = 0)
		: Type(type)
		, Event(event)
		, LabelIndex(labelIndex)
	{
	}

	inline LightPathToken(MaterialScatteringType type, uint16 labelIndex = 0)
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

	inline uint32 toPacked() const
	{
		_LightPathTokenPacked packed;
		packed._S.T = Type;
		packed._S.E = Event;
		packed._S.L = LabelIndex;
		return packed._I;
	}

	static inline LightPathToken fromPacked(uint32 p)
	{
		_LightPathTokenPacked packed;
		packed._I = p;
		return LightPathToken((ScatteringType)packed._S.T, (ScatteringEvent)packed._S.E, packed._S.L);
	}

	static inline LightPathToken Camera() { return LightPathToken(ST_CAMERA, SE_NONE); }
	static inline LightPathToken Background() { return LightPathToken(ST_BACKGROUND, SE_NONE); }
	static inline LightPathToken Emissive() { return LightPathToken(ST_EMISSIVE, SE_NONE); }
};
} // namespace PR