#include "ImageSpectralOutput.h"

#include "shader/SamplePoint.h"
#include "Logger.h"

#include "spectral/RGBConverter.h"

using namespace PR;
namespace PRU
{
	ImageSpectralShaderOutput::ImageSpectralShaderOutput(OIIO::TextureSystem* tsys, const OIIO::TextureOpt& options, const std::string& filename) :
		SpectralShaderOutput(), mFilename(filename), mTextureOptions(options), mTextureSystem(tsys)
	{
		PR_ASSERT(mTextureSystem);
		PR_ASSERT(!mFilename.empty());
	}

	PR::Spectrum ImageSpectralShaderOutput::eval(const PR::SamplePoint& point)
	{
		PR_LOGGER.logf(L_Warning, M_Scene, "TEST2 %s", mFilename.c_str());
		float res[3] = { 0, 0, 0 };

		auto thread_info = mTextureSystem->get_perthread_info();
		auto file_handle = mTextureSystem->get_texture_handle(mFilename, thread_info);
		PR_LOGGER.logf(L_Warning, M_Scene, "TEST3 %p", file_handle);
		if (!mTextureSystem->texture(file_handle, thread_info, mTextureOptions,
			PM::pm_GetX(point.UV), 1-PM::pm_GetY(point.UV),
			0, 0, 0, 0,
			3, &res[0]))
		{
			PR_LOGGER.log(L_Warning, M_Scene, "TESTff");
			std::string err = mTextureSystem->geterror();
			PR_LOGGER.logf(L_Error, M_Scene, "Couldn't lookup texture at UV [%f, %f]: %s",
				PM::pm_GetX(point.UV), 1-PM::pm_GetY(point.UV), err.c_str());
		}
		PR_LOGGER.log(L_Warning, M_Scene, "TEST");
		
		return RGBConverter::toSpec(res[0], res[1], res[2]);
	}
}