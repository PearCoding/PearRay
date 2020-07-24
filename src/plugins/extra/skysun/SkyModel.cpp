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
	mPhiCount	= params.getInt("phi_resolution", RES_PHI);
	mThetaCount = params.getInt("theta_resolution", RES_THETA);

	const float solar_elevation		  = 0.5f * PR_PI - sunEA.Elevation;
	const float atmospheric_turbidity = params.getNumber("turbidity", 3.0f);

	mData.resize(mPhiCount * mThetaCount * AR_SPECTRAL_BANDS);
	for (size_t k = 0; k < AR_SPECTRAL_BANDS; ++k) {
		float wavelength = AR_SPECTRAL_START + k * AR_SPECTRAL_DELTA;
		// Evaluate albedo for particular wavelength -> Does not support textures etc
		ShadingContext ctx;
		ctx.WavelengthNM = SpectralBlob(wavelength);

		const float albedo = ground_albedo->eval(ctx)[0];

		auto* state = arhosekskymodelstate_alloc_init(solar_elevation, atmospheric_turbidity, albedo);
		auto body	= [&](const tbb::blocked_range2d<size_t, size_t>& r) {
			  for (size_t y = r.rows().begin(); y != r.rows().end(); ++y) {
				  const float theta = PR_PI * y / (float)mThetaCount;
				  if (theta >= PR_PI / 2) {
					  for (size_t x = r.cols().begin(); x != r.cols().end(); ++x) {
						  mData[y * mPhiCount * AR_SPECTRAL_BANDS + x * AR_SPECTRAL_BANDS + k] = 0;
					  }
				  } else {
					  for (size_t x = r.cols().begin(); x != r.cols().end(); ++x) {
						  const float phi = 2 * PR_PI * x / (float)mPhiCount;

						  ElevationAzimuth ea = ElevationAzimuth::fromThetaPhi(theta, phi);
						  float cosGamma	  = std::cos(theta) * std::cos(sunEA.Elevation)
										   + std::sin(theta) * std::sin(sunEA.Elevation) * std::cos(ea.Azimuth - sunEA.Azimuth);
						  float gamma	 = std::acos(std::min(1.0f, std::max(-1.0f, cosGamma)));
						  float radiance = arhosekskymodel_radiance(state, theta, gamma, wavelength + 0.005f /*Make sure the correct bin is choosen*/);

						  mData[y * mPhiCount * AR_SPECTRAL_BANDS + x * AR_SPECTRAL_BANDS + k] = std::max(0.0f, radiance);
					  }
				  }
			  }
		};
		tbb::parallel_for(tbb::blocked_range2d<size_t, size_t>(0, mThetaCount, 0, mPhiCount), body);

		arhosekskymodelstate_free(state);
	}

	Debug::saveImage("sky.exr", mData.data(), mPhiCount, mThetaCount, AR_SPECTRAL_BANDS);
}
} // namespace PR