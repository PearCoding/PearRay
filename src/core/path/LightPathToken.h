#pragma once

#include "material/MaterialType.h"

namespace PR {
enum class ScatteringType : uint8 {
	Camera = 0,
	Emissive,
	Refraction,
	Reflection,
	Background,
	_COUNT
};

enum class ScatteringEvent : uint8 {
	Diffuse = 0,
	Specular,
	None,
	_COUNT
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

	inline LightPathToken(ScatteringType type = ScatteringType::Camera, ScatteringEvent event = ScatteringEvent::None, uint16 labelIndex = 0)
		: Type(type)
		, Event(event)
		, LabelIndex(labelIndex)
	{
	}

	inline LightPathToken(MaterialScatteringType type, uint16 labelIndex = 0)
		: LabelIndex(labelIndex)
	{
		switch (type) {
		case MaterialScatteringType::DiffuseReflection:
			Type  = ScatteringType::Reflection;
			Event = ScatteringEvent::Diffuse;
			break;
		case MaterialScatteringType::DiffuseTransmission:
			Type  = ScatteringType::Refraction;
			Event = ScatteringEvent::Diffuse;
			break;
		case MaterialScatteringType::SpecularReflection:
			Type  = ScatteringType::Reflection;
			Event = ScatteringEvent::Specular;
			break;
		case MaterialScatteringType::SpecularTransmission:
			Type  = ScatteringType::Refraction;
			Event = ScatteringEvent::Specular;
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
		packed._S.T = (uint8)Type;
		packed._S.E = (uint8)Event;
		packed._S.L = LabelIndex;
		return packed._I;
	}

	static inline LightPathToken fromPacked(uint32 p)
	{
		_LightPathTokenPacked packed;
		packed._I = p;
		return LightPathToken((ScatteringType)packed._S.T, (ScatteringEvent)packed._S.E, packed._S.L);
	}

	static inline LightPathToken Camera() { return LightPathToken(ScatteringType::Camera, ScatteringEvent::None); }
	static inline LightPathToken Background() { return LightPathToken(ScatteringType::Background, ScatteringEvent::None); }
	static inline LightPathToken Emissive() { return LightPathToken(ScatteringType::Emissive, ScatteringEvent::None); }
};
} // namespace PR