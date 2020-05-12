#include "SocketParser.h"
#include "Environment.h"
#include "Logger.h"
#include "MathParser.h"
#include "shader/ConstSocket.h"

#include "DataLisp.h"

namespace PR {

std::shared_ptr<FloatSpectralShadingSocket> SocketParser::getSpectralOutput(Environment* env, const DL::Data& dataD, bool allowScalar)
{
	if (allowScalar && dataD.isNumber()) {
		return std::make_shared<ConstSpectralShadingSocket>(
			Spectrum(env->spectrumDescriptor(), dataD.getNumber()));
	} else if (dataD.type() == DL::DT_String) {
		if (env->hasSpectrum(dataD.getString()))
			return std::make_shared<ConstSpectralShadingSocket>(env->getSpectrum(dataD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find spectrum " << dataD.getString() << " for material" << std::endl;
	} else if (dataD.type() == DL::DT_Group) {
		std::string name = dataD.getGroup().id();

		if ((name == "tex" || name == "texture") && dataD.getGroup().anonymousCount() == 1) {
			DL::Data nameD = dataD.getGroup().at(0);
			if (nameD.type() == DL::DT_String) {
				if (env->isShadingSocket<FloatSpectralShadingSocket>(nameD.getString()))
					return env->getShadingSocket<FloatSpectralShadingSocket>(nameD.getString());
				else
					PR_LOG(L_WARNING) << "Unknown spectral texture " << nameD.getString() << "." << std::endl;
			}
		} else {
			PR_LOG(L_WARNING) << "Unknown data entry." << std::endl;
		}
	} else if (dataD.isValid()) {
		PR_LOG(L_WARNING) << "Unknown texture entry." << std::endl;
	}

	return nullptr;
}

std::shared_ptr<FloatScalarShadingSocket> SocketParser::getScalarOutput(Environment* env, const DL::Data& dataD)
{
	if (dataD.isNumber()) {
		return std::make_shared<ConstScalarShadingSocket>(dataD.getNumber());
	} else if (dataD.type() == DL::DT_Group) {
		std::string name = dataD.getGroup().id();

		if ((name == "tex" || name == "texture") && dataD.getGroup().anonymousCount() == 1) {
			DL::Data nameD = dataD.getGroup().at(0);
			if (nameD.type() == DL::DT_String) {
				if (env->isShadingSocket<FloatScalarShadingSocket>(nameD.getString()))
					return env->getShadingSocket<FloatScalarShadingSocket>(nameD.getString());
				else
					PR_LOG(L_WARNING) << "Unknown scalar texture " << nameD.getString() << "." << std::endl;
			}
		} else {
			PR_LOG(L_WARNING) << "Unknown data entry." << std::endl;
		}
	} else if (dataD.isValid()) {
		PR_LOG(L_WARNING) << "Unknown texture entry." << std::endl;
	}

	return nullptr;
}

std::shared_ptr<FloatVectorShadingSocket> SocketParser::getVectorOutput(Environment* env, const DL::Data& dataD)
{
	if (dataD.type() == DL::DT_Group) {
		if (dataD.getGroup().isArray()) {
			bool ok;
			const auto vec = MathParser::getVector(dataD.getGroup(), ok);

			if (ok) {
				return std::make_shared<ConstVectorShadingSocket>(vec);
			} else {
				PR_LOG(L_WARNING) << "Invalid vector entry." << std::endl;
			}
		} else {
			std::string name = dataD.getGroup().id();

			if ((name == "tex" || name == "texture") && dataD.getGroup().anonymousCount() == 1) {
				DL::Data nameD = dataD.getGroup().at(0);
				if (nameD.type() == DL::DT_String) {
					if (env->isShadingSocket<FloatVectorShadingSocket>(nameD.getString()))
						return env->getShadingSocket<FloatVectorShadingSocket>(nameD.getString());
					else
						PR_LOG(L_WARNING) << "Unknown vector texture " << nameD.getString() << std::endl;
				}
			} else {
				PR_LOG(L_WARNING) << "Unknown data entry." << std::endl;
			}
		}
	} else if (dataD.isValid()) {
		PR_LOG(L_WARNING) << "Unknown texture entry." << std::endl;
	}

	return nullptr;
}
} // namespace PR
