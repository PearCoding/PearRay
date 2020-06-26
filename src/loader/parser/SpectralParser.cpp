#include "SpectralParser.h"
#include "Logger.h"
#include "shader/ConstNode.h"
#include "spectral/SpectralUpsampler.h"

#include "DataLisp.h"

namespace PR {

std::shared_ptr<FloatSpectralNode> SpectralParser::getSpectrum(SpectralUpsampler* upsampler, const DL::Data& dataD)
{
	PR_ASSERT(upsampler, "Expected valid upsampler");

	if (dataD.type() == DL::DT_Group) {
		DL::DataGroup grp = dataD.getGroup();
		if (grp.id() == "field" || grp.id() == "parametric") {
			ParametricBlob blob;
			DL::Data defaultD = grp.getFromKey("default");

			if (defaultD.isNumber()) {
				for (uint32 i = 0; i < PR_PARAMETRIC_BLOB_SIZE; ++i)
					blob[i] = defaultD.getNumber();
			}

			for (uint32 i = 0;
				 i < grp.anonymousCount() && i < PR_PARAMETRIC_BLOB_SIZE;
				 ++i) {
				DL::Data fieldD = grp.at(i);

				if (fieldD.isNumber())
					blob[i] = fieldD.getNumber();
			}
			return std::make_shared<ParametricSpectralNode>(blob);
		} else if (grp.id() == "illum" || grp.id() == "illumination") {
			ParametricBlob input;
			if (grp.anonymousCount() == PR_PARAMETRIC_BLOB_SIZE
				&& grp.isAllAnonymousNumber()) {
				for (size_t i = 0; i < PR_PARAMETRIC_BLOB_SIZE; ++i)
					input[i] = grp.at(i).getNumber();
			}

			ParametricBlob blob;
			const float max = input.maxCoeff();
			float power		= 1;
			if (max <= 0.0f) {
				upsampler->prepare(&input(0), &input(1), &input(2), &blob(0), &blob(1), &blob(2), 1);
				power = 1;
			} else {
				const float scale		 = 2 * max;
				ParametricBlob scaled_in = input / scale;
				upsampler->prepare(&scaled_in(0), &scaled_in(1), &scaled_in(2), &blob(0), &blob(1), &blob(2), 1);
				power = scale;
			}
			return std::make_shared<ParametricScaledSpectralNode>(blob, power);
		} else if (grp.id() == "refl" || grp.id() == "reflective") {
			ParametricBlob input;
			if (grp.anonymousCount() == PR_PARAMETRIC_BLOB_SIZE
				&& grp.isAllAnonymousNumber()) {
				for (size_t i = 0; i < PR_PARAMETRIC_BLOB_SIZE; ++i)
					input[i] = grp.at(i).getNumber();
			}

			const float max = input.maxCoeff();
			if (max > 1.0f)
				PR_LOG(L_WARNING) << "Given reflective rgb " << input << " contains coefficients above 1" << std::endl;

			ParametricBlob blob;
			upsampler->prepare(&input(0), &input(1), &input(2), &blob(0), &blob(1), &blob(2), 1);
			return std::make_shared<ParametricSpectralNode>(blob);
		} else {
			PR_LOG(L_WARNING) << "Couldn't construct spectrum. Unknown type "
							  << grp.id() << std::endl;
		}
	}

	return nullptr;
}
} // namespace PR
