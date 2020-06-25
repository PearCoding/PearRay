#include "SocketParser.h"
#include "Environment.h"
#include "Logger.h"
#include "MathParser.h"
#include "shader/ConstNode.h"

#include "DataLisp.h"

namespace PR {

std::shared_ptr<FloatSpectralNode> SocketParser::getSpectralOutput(Environment* env, const DL::Data& dataD, bool allowScalar)
{
	if (allowScalar && dataD.isNumber()) {
		return std::make_shared<ConstSpectralNode>(ParametricBlob(dataD.getNumber()));
	} else if (dataD.type() == DL::DT_String) {
		if (env->hasSpectrum(dataD.getString()))
			return std::make_shared<ConstSpectralNode>(env->getSpectrum(dataD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find spectrum " << dataD.getString() << " for material" << std::endl;
	} else if (dataD.type() == DL::DT_Group) {
		std::string name = dataD.getGroup().id();

		if ((name == "tex" || name == "texture") && dataD.getGroup().anonymousCount() == 1) {
			DL::Data nameD = dataD.getGroup().at(0);
			if (nameD.type() == DL::DT_String) {
				if (env->isNode<FloatSpectralNode>(nameD.getString()))
					return env->getNode<FloatSpectralNode>(nameD.getString());
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

std::shared_ptr<FloatScalarNode> SocketParser::getScalarOutput(Environment* env, const DL::Data& dataD)
{
	if (dataD.isNumber()) {
		return std::make_shared<ConstScalarNode>(dataD.getNumber());
	} else if (dataD.type() == DL::DT_Group) {
		std::string name = dataD.getGroup().id();

		if ((name == "tex" || name == "texture") && dataD.getGroup().anonymousCount() == 1) {
			DL::Data nameD = dataD.getGroup().at(0);
			if (nameD.type() == DL::DT_String) {
				if (env->isNode<FloatScalarNode>(nameD.getString()))
					return env->getNode<FloatScalarNode>(nameD.getString());
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

std::shared_ptr<FloatVectorNode> SocketParser::getVectorOutput(Environment* env, const DL::Data& dataD)
{
	if (dataD.type() == DL::DT_Group) {
		if (dataD.getGroup().isArray()) {
			bool ok;
			const auto vec = MathParser::getVector(dataD.getGroup(), ok);

			if (ok) {
				return std::make_shared<ConstVectorNode>(vec);
			} else {
				PR_LOG(L_WARNING) << "Invalid vector entry." << std::endl;
			}
		} else {
			std::string name = dataD.getGroup().id();

			if ((name == "tex" || name == "texture") && dataD.getGroup().anonymousCount() == 1) {
				DL::Data nameD = dataD.getGroup().at(0);
				if (nameD.type() == DL::DT_String) {
					if (env->isNode<FloatVectorNode>(nameD.getString()))
						return env->getNode<FloatVectorNode>(nameD.getString());
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
