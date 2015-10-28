#pragma once

#include "MeshLoader.h"

namespace PR
{
	class WavefrontLoader : public MeshLoader
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

		void load(const std::string& file, Mesh* mesh);

	private:
		float mScale;
	};
}