#include "SpectralParser.h"
#include "Logger.h"
#include "spectral/SpectralUpsampler.h"

#include "DataLisp.h"

namespace PR {

ParametricBlob SpectralParser::getSpectrum(SpectralUpsampler* upsampler, const DL::Data& dataD)
{
	ParametricBlob blob;
	if (dataD.type() == DL::DT_Group) {
		DL::DataGroup grp = dataD.getGroup();
		if (grp.id() == "field" || grp.id() == "parametric") {
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
		} else if (grp.id() == "illum" || grp.id() == "illumination") {
			ParametricBlob input;
			if (grp.anonymousCount() == PR_PARAMETRIC_BLOB_SIZE
				&& grp.isAllAnonymousNumber()) {
				for (size_t i = 0; i < PR_PARAMETRIC_BLOB_SIZE; ++i)
					blob[i] = grp.at(i).getNumber();
			}

			upsampler->prepare(&input(0), &input(1), &input(2), &blob(0), &blob(1), &blob(2), 1);
		} else if (grp.id() == "refl" || grp.id() == "reflective") {
			ParametricBlob input;
			if (grp.anonymousCount() == PR_PARAMETRIC_BLOB_SIZE
				&& grp.isAllAnonymousNumber()) {
				for (size_t i = 0; i < PR_PARAMETRIC_BLOB_SIZE; ++i)
					input[i] = grp.at(i).getNumber();
			}

			upsampler->prepare(&input(0), &input(1), &input(2), &blob(0), &blob(1), &blob(2), 1);
		} else {
			PR_LOG(L_WARNING) << "Couldn't construct spectrum. Unknown type "
							  << grp.id() << std::endl;
		}
	}

	return blob;
}
} // namespace PR
