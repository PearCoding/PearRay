#include "ImageLoader.h"
#include "PearPic.h"
#include "Image.h"

#include "Logger.h"

#include "texture/RGBTexture2D.h"
#include "texture/SpecTexture2D.h"

// Slow implementation, and needs more protections and error messages!
using namespace PR;
namespace PRU
{
	ImageLoader::ImageLoader()
	{
		mPearPic = new PP::PearPic();
		mPearPic->init();
	}

	ImageLoader::~ImageLoader()
	{
		if (mPearPic)
		{
			mPearPic->exit();
			delete mPearPic;
		}
	}

	Texture2D* ImageLoader::load(const std::string& filename) const
	{
		PP::Image image = mPearPic->load(filename);
		if (image.isValid())
		{
			image = image.convertTo(PP::CF_RGBA, PP::CP_Byte);

			if (image.isValid())
			{
				float* data = new float[image.width()*image.height() * 4];
				for (uint32 i = 0; i < image.width(); ++i)
				{
					for (uint32 j = 0; j < image.height(); ++j)
					{
						data[j * image.width() * 4 + i * 4] = image.data()[j * image.width() * 4 + i * 4] / 255.0f;
						data[j * image.width() * 4 + i * 4 + 1] = image.data()[j * image.width() * 4 + i * 4 + 1] / 255.0f;
						data[j * image.width() * 4 + i * 4 + 2] = image.data()[j * image.width() * 4 + i * 4 + 2] / 255.0f;
						data[j * image.width() * 4 + i * 4 + 3] = image.data()[j * image.width() * 4 + i * 4 + 3] / 255.0f;
					}
				}

				return new RGBTexture2D(data, image.width(), image.height());
			}
		}
		else
		{
			// TODO: Try spectral file format
		}

		PR_LOGGER.logf(PR::L_Error, PR::M_Scene, "Couldn't load texture '%s'", filename.c_str());
		return nullptr;
	}
}