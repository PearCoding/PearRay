#pragma once

#include "SubGraphLoader.h"
#include <map>

namespace PR
{
	class PR_LIB_UTILS WavefrontLoader : public SubGraphLoader
	{
	public:
		WavefrontLoader(const std::map<std::string, std::string>& overrides);
		~WavefrontLoader();

		inline float scale() const
		{
			return mScale;
		}

		inline void setScale(float f)
		{
			mScale = f;
		}

		inline void flipNormal(bool b)
		{
			mFlipNormal = b;
		}

		void load(const std::string& file, Environment* env) override;

	private:
		std::map<std::string, std::string> mOverrides;
		float mScale;
		bool mFlipNormal;
	};
}