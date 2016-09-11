#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	struct ShaderClosure;
	class Spectrum;

	/*
	 * Representing infinite lights
	 * like distant lights or backgrounds
	 */
	class PR_LIB_INLINE IInfiniteLight
	{
	public:
		IInfiniteLight() {}
		virtual ~IInfiniteLight() {}

		virtual float pdf(const PM::vec3& L) = 0;
		virtual PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) = 0;
		virtual Spectrum apply(const PM::vec3& L) = 0;
	};
}