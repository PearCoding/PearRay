#include "ImageLoader.h"

#include "Logger.h"

#include "texture/RGBTexture2D.h"
#include "texture/SpecTexture2D.h"

#include <FreeImage.h>

using namespace PR;
namespace PRU
{
	void FreeImage_ErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
		std::string str = "FreeImage: ";
		str += message;
		PR_LOGGER.log(L_Error, M_Internal, str);
	}

	ImageLoader::ImageLoader()
	{
#ifdef FREEIMAGE_LIB
		FreeImage_Initialise();// Only when linking static
#endif
		FreeImage_SetOutputMessage(FreeImage_ErrorHandler);
	}

	ImageLoader::~ImageLoader()
	{
#ifdef FREEIMAGE_LIB
		FreeImage_DeInitialise();
#endif
	}

	Texture2D* ImageLoader::load(const std::string& filename) const
	{
		FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename.c_str());

		if (format == FIF_UNKNOWN)
			format = FreeImage_GetFIFFromFilename(filename.c_str());

		if (format != FIF_UNKNOWN)
		{
			if (FreeImage_FIFSupportsReading(format))
			{
				FIBITMAP* image = FreeImage_Load(format, filename.c_str(), 0);
				
				if (image)
				{
					const uint32 width = FreeImage_GetWidth(image);
					const uint32 height = FreeImage_GetHeight(image);
					const uint32 pitch = FreeImage_GetPitch(image);
					BYTE* bits = (BYTE*)FreeImage_GetBits(image);

					if (width == 0 || height == 0 || bits == nullptr)
					{
						FreeImage_Unload(image);
						PR_LOGGER.logf(PR::L_Error, PR::M_Scene, "Invalid file '%'", filename.c_str());
						return nullptr;
					}

					FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(image);

					if (image_type == FIT_RGBF)
					{
						float* data = new float[width*height * 4];
						for (uint32 j = 0; j < height; ++j)
						{
							FIRGBF * pixel = (FIRGBF*)bits;
							for (uint32 i = 0; i < width; ++i)
							{
								data[j * width * 4 + i * 4] = pixel[i].red;
								data[j * width * 4 + i * 4 + 1] = pixel[i].green;
								data[j * width * 4 + i * 4 + 2] = pixel[i].blue;
								data[j * width * 4 + i * 4 + 3] = 1;
							}
							bits += pitch;
						}

						FreeImage_Unload(image);
						return new RGBTexture2D(data, width, height);
					}
					else if (image_type == FIT_RGBAF)
					{
						float* data = new float[width*height * 4];
						for (uint32 j = 0; j < height; ++j)
						{
							FIRGBAF * pixel = (FIRGBAF*)bits;
							for (uint32 i = 0; i < width; ++i)
							{
								data[j * width * 4 + i * 4] = pixel[i].red;
								data[j * width * 4 + i * 4 + 1] = pixel[i].green;
								data[j * width * 4 + i * 4 + 2] = pixel[i].blue;
								data[j * width * 4 + i * 4 + 3] = pixel[i].alpha;
							}
							bits += pitch;
						}

						FreeImage_Unload(image);
						return new RGBTexture2D(data, width, height);
					}
					else
					{
						if (FreeImage_GetBPP(image) != 32)
						{
							FIBITMAP* dst = FreeImage_ConvertTo32Bits(image);
							FreeImage_Unload(image);
							image = dst;
						}

						if (image)
						{
							PR_ASSERT(FreeImage_GetBPP(image) == 32);
							bits = FreeImage_GetBits(image);

							if (bits == nullptr)
							{
								FreeImage_Unload(image);
								PR_LOGGER.logf(PR::L_Error, PR::M_Scene, "Invalid file '%'", filename.c_str());
								return nullptr;
							}

							float* data = new float[width*height * 4];

							constexpr float inv = 1 / 255.0f;

							for (uint32 j = 0; j < height; ++j)
							{
								BYTE* pixel = bits;
								for (uint32 i = 0; i < width; ++i)
								{
									data[j * width * 4 + i * 4] = pixel[FI_RGBA_RED] * inv;
									data[j * width * 4 + i * 4 + 1] = pixel[FI_RGBA_GREEN] * inv;
									data[j * width * 4 + i * 4 + 2] = pixel[FI_RGBA_BLUE] * inv;
									data[j * width * 4 + i * 4 + 3] = pixel[FI_RGBA_ALPHA] * inv;
									pixel += 4;
								}
								bits += pitch;
							}

							FreeImage_Unload(image);
							return new RGBTexture2D(data, width, height);
						}
					}
				}
			}
			else
			{
				PR_LOGGER.logf(PR::L_Error, PR::M_Scene, "FreeImage doesn't support reading '%s'",
					FreeImage_GetFIFMimeType(format));
			}
		}

		PR_LOGGER.logf(PR::L_Error, PR::M_Scene, "Couldn't load texture '%s'", filename.c_str());
		return nullptr;
	}
}