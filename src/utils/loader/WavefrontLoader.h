#pragma once

#include "MeshLoader.h"

namespace PRU
{
	class PR_LIB_UTILS WavefrontLoader : public MeshLoader
	{
	public:
		WavefrontLoader();
		~WavefrontLoader();

		inline float scale() const
		{
			return mScale;
		}

		inline void setScale(float f)
		{
			mScale = f;
		}

		void load(const std::string& file, PR::Mesh* mesh);

	private:
		float mScale;
	};
}