#pragma once

#include "PR_Config.h"
#include "PearMath.h"

namespace PR
{
	struct ShaderClosure;
	class Spectrum;

	/*
	 * Representing infinite lights
	 * like distant lights or backgrounds
	 */
	class PR_LIB IInfiniteLight
	{
	public:
		IInfiniteLight() : mFrozen(false) {}
		virtual ~IInfiniteLight() {}

		virtual PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) = 0;
		virtual Spectrum apply(const PM::vec3& V) = 0;

		inline void freeze()
		{
			if(!mFrozen)
			{
				onFreeze();
				mFrozen = true;
			}
		}

		inline bool isFrozen() const
		{
			return mFrozen;
		}

		virtual void onFreeze() {};

	private:
		bool mFrozen;
	};
}
