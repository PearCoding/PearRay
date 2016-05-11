#pragma once

#include <string>
#include "texture/Texture2D.h"

namespace PP
{
	class PearPic;
}

namespace PRU
{
	class PR_LIB_UTILS ImageLoader
	{
	public:
		ImageLoader();
		~ImageLoader();

		PR::Texture2D* load(const std::string& filename) const;

	private:
		PP::PearPic* mPearPic;
	};
}