#include "SkyModel.h"
#include "SunLocation.h"

#include "SceneLoadContext.h"
#include "shader/INode.h"
#include "shader/ShadingContext.h"
#include "skymodel/ArHosekSkyModel.h"

#include "DebugIO.h"

#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>

namespace PR {
SkyModel::SkyModel(const std::shared_ptr<FloatSpectralNode>& ground_albedo, const ElevationAzimuth& sunEA, const ParameterGroup& params)
{
	mAzimuthCount	= params.getInt("azimuth_resolution", RES_AZ);
	mElevationCount = params.getInt("elevation_resolution", RES_EL);

	const float solar_elevation		  = PR_PI / 2 - sunEA.Elevation;
	const float atmospheric_turbidity = params.getNumber("turbidity", 3.0f);

	const float sun_se = std::sin(solar_elevation);
	const float sun_ce = std::cos(solar_elevation);

	mData.resize(mElevationCount * mAzimuthCount * AR_SPECTRAL_BANDS);
	for (size_t k = 0; k < AR_SPECTRAL_BANDS; ++k) {
		float wavelength = AR_SPECTRAL_START + k * AR_SPECTRAL_DELTA;
		// Evaluate albedo for particular wavelength -> Does not support textures etc
		ShadingContext ctx;
		ctx.WavelengthNM = SpectralBlob(wavelength);

		const float albedo = ground_albedo->eval(ctx)[0];

		auto* state = arhosekskymodelstate_alloc_init(solar_elevation, atmospheric_turbidity, albedo);
		auto body	= [&](const tbb::blocked_range2d<size_t, size_t>& r) {
			  for (size_t y = r.rows().begin(); y != r.rows().end(); ++y) {
				  const float theta = PR_PI / 2 - ELEVATION_RANGE * y / (float)mElevationCount;
				  const float st	= std::sin(theta);
				  const float ct	= std::cos(theta);
				  for (size_t x = r.cols().begin(); x != r.cols().end(); ++x) {
					  const float azimuth = AZIMUTH_RANGE * x / (float)mAzimuthCount;

					  float cosGamma = ct * sun_ce + st * sun_se * std::cos(azimuth - sunEA.Azimuth);
					  float gamma	 = std::acos(std::min(1.0f, std::max(-1.0f, cosGamma)));
					  float radiance = arhosekskymodel_radiance(state, theta, gamma, wavelength + 0.005f /*Make sure the correct bin is choosen*/);

					  mData[y * mAzimuthCount * AR_SPECTRAL_BANDS + x * AR_SPECTRAL_BANDS + k] = std::max(0.0f, radiance);
				  }
			  }
		};
		tbb::parallel_for(tbb::blocked_range2d<size_t, size_t>(0, mElevationCount, 0, mAzimuthCount), body);

		arhosekskymodelstate_free(state);
	}

#if 0
	Debug::saveImage("sky.exr", mData.data(), mAzimuthCount, mElevationCount, AR_SPECTRAL_BANDS);
#endif
}
} // namespace PR