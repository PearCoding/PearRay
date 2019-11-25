#include "SpectralParser.h"
#include "Logger.h"
#include "spectral/RGBConverter.h"
#include "spectral/XYZConverter.h"

#include "DataLisp.h"

namespace PR {

Spectrum SpectralParser::getSpectrum(const std::shared_ptr<SpectrumDescriptor>& desc, const DL::Data& dataD)
{
	Spectrum spec(desc);
	if (dataD.type() == DL::DT_Group) {
		DL::DataGroup grp = dataD.getGroup();
		if (grp.isArray()) {
			for (size_t i = 0; i < grp.anonymousCount() && i < spec.samples(); ++i) {
				if (grp.at(i).isNumber())
					spec.setValue(i, grp.at(i).getNumber());
				else
					PR_LOG(L_WARNING) << "Couldn't set spectrum entry at index " << i << std::endl;
			}
		} else if (grp.id() == "field") {
			DL::Data defaultD = grp.getFromKey("default");

			if (defaultD.isNumber()) {
				for (uint32 i = 0; i < spec.samples(); ++i) {
					spec.setValue(i, defaultD.getNumber());
				}
			}

			for (uint32 i = 0;
				 i < grp.anonymousCount() && i < spec.samples();
				 ++i) {
				DL::Data fieldD = grp.at(i);

				if (fieldD.isNumber())
					spec.setValue(i, fieldD.getNumber());
			}
		} else if (grp.id() == "rgb") {
			if (grp.anonymousCount() == 3
				&& grp.isAllAnonymousNumber()) {
				RGBConverter::toSpec(spec,
									 grp.at(0).getNumber(),
									 grp.at(1).getNumber(),
									 grp.at(2).getNumber());
			}
		} else if (grp.id() == "xyz") {
			if (grp.anonymousCount() == 3
				&& grp.isAllAnonymousNumber()) {
				XYZConverter::toSpec(spec,
									 grp.at(0).getNumber(),
									 grp.at(1).getNumber(),
									 grp.at(2).getNumber());
			}
		} else if (grp.id() == "temperature" || grp.id() == "blackbody") { // Luminance
			if (grp.anonymousCount() >= 1 && grp.at(0).isNumber()) {
				// TODO: Should use sRGB, then convert to standard spec
				spec = Spectrum::blackbody(spec.descriptor(), std::max(0.0f, grp.at(0).getNumber()));
				spec.weightPhotometric();
			}

			if (grp.anonymousCount() >= 2 && grp.at(1).isNumber()) {
				spec *= grp.at(1).getNumber();
			}
		} else if (grp.id() == "temperature_raw" || grp.id() == "blackbody_raw") { // Radiance
			if (grp.anonymousCount() >= 1 && grp.at(0).isNumber()) {
				// TODO: Should use sRGB, then convert to standard spec
				spec = Spectrum::blackbody(spec.descriptor(), std::max(0.0f, grp.at(0).getNumber()));
			}

			if (grp.anonymousCount() >= 2 && grp.at(1).isNumber()) {
				spec *= grp.at(1).getNumber();
			}
		} else if (grp.id() == "temperature_norm" || grp.id() == "blackbody_norm") { // Luminance Norm
			if (grp.anonymousCount() >= 1 && grp.at(0).isNumber()) {
				// TODO: Should use sRGB, then convert to standard spec
				spec = Spectrum::blackbody(spec.descriptor(), std::max(0.0f, grp.at(0).getNumber()));
				spec.weightPhotometric();
				spec.normalize();
			}

			if (grp.anonymousCount() >= 2 && grp.at(1).isNumber()) {
				spec *= grp.at(1).getNumber();
			}
		} else if (grp.id() == "temperature_raw_norm" || grp.id() == "blackbody_raw_norm") { // Radiance Norm
			if (grp.anonymousCount() >= 1 && grp.at(0).isNumber()) {
				// TODO: Should use sRGB, then convert to standard spec
				spec = Spectrum::blackbody(spec.descriptor(), std::max(0.0f, grp.at(0).getNumber()));
				spec.normalize();
			}

			if (grp.anonymousCount() >= 2 && grp.at(1).isNumber()) {
				spec *= grp.at(1).getNumber();
			}
		}
	}

	return spec;
}
} // namespace PR