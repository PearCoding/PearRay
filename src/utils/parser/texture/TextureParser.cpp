#include "TextureParser.h"
#include "Environment.h"
#include "Logger.h"
#include "SceneLoader.h"

#include "shader/ImageMapSocket.h"

#include "DataLisp.h"

#include <algorithm>

#include <boost/filesystem.hpp>

namespace PR {
OIIO::TextureOpt::Wrap parseWrap(const std::string& name)
{
	if (name == "black")
		return OIIO::TextureOpt::WrapBlack;
	else if (name == "clamp")
		return OIIO::TextureOpt::WrapClamp;
	else if (name == "periodic")
		return OIIO::TextureOpt::WrapPeriodic;
	else if (name == "mirror")
		return OIIO::TextureOpt::WrapMirror;
	else
		return OIIO::TextureOpt::WrapDefault;
}

OIIO::TextureOpt::MipMode parseMIP(const std::string& name)
{
	if (name == "none")
		return OIIO::TextureOpt::MipModeNoMIP;
	else if (name == "one" || name == "linear")
		return OIIO::TextureOpt::MipModeOneLevel;
	else if (name == "trilinear" || name == "tri")
		return OIIO::TextureOpt::MipModeTrilinear;
	else if (name == "anisotropic" || name == "aniso")
		return OIIO::TextureOpt::MipModeAniso;
	else
		return OIIO::TextureOpt::MipModeDefault;
}

OIIO::TextureOpt::InterpMode parseInterpolation(const std::string& name)
{
	if (name == "closest")
		return OIIO::TextureOpt::InterpClosest;
	else if (name == "bi" || name == "bilinear")
		return OIIO::TextureOpt::InterpBilinear;
	else if (name == "bicubic")
		return OIIO::TextureOpt::InterpBicubic;
	else
		return OIIO::TextureOpt::InterpSmartBicubic;
}

void TextureParser::parse(Environment* env, const std::string& name, const DL::DataGroup& group) const
{
	DL::Data filenameD = group.getFromKey("file");
	DL::Data typeD	 = group.getFromKey("type");
	//DL::Data dimD = group.getFromKey("dimension");
	DL::Data wrapModeD			= group.getFromKey("wrap");
	DL::Data mipModeD			= group.getFromKey("mip");
	DL::Data interpolationModeD = group.getFromKey("interpolation");
	DL::Data blurD				= group.getFromKey("blur");
	DL::Data anisoD				= group.getFromKey("anisotropic");

	OIIO::TextureOpt opts;
	if (wrapModeD.type() == DL::Data::T_String) {
		std::string wrap = wrapModeD.getString();
		std::transform(wrap.begin(), wrap.end(), wrap.begin(), ::tolower);
		opts.swrap = parseWrap(wrap);
		opts.twrap = parseWrap(wrap);
		opts.rwrap = parseWrap(wrap);
	} else if (wrapModeD.type() == DL::Data::T_Group) {
		DL::DataGroup arr = wrapModeD.getGroup();
		for (uint32 i = 0; i < 3 && i < arr.anonymousCount(); ++i) {
			DL::Data dat = arr.at(i);
			if (dat.type() == DL::Data::T_String) {
				std::string wrap = dat.getString();
				std::transform(wrap.begin(), wrap.end(), wrap.begin(), ::tolower);
				switch (i) {
				case 0:
					opts.swrap = parseWrap(wrap);
					break;
				case 1:
					opts.twrap = parseWrap(wrap);
					break;
				case 2:
					opts.rwrap = parseWrap(wrap);
					break;
				}
			}
		}
	}

	if (mipModeD.type() == DL::Data::T_String) {
		std::string mip = mipModeD.getString();
		std::transform(mip.begin(), mip.end(), mip.begin(), ::tolower);
		opts.mipmode = parseMIP(mip);
	}

	if (interpolationModeD.type() == DL::Data::T_String) {
		std::string interp = interpolationModeD.getString();
		std::transform(interp.begin(), interp.end(), interp.begin(), ::tolower);
		opts.interpmode = parseInterpolation(interp);
	}

	if (blurD.isNumber()) {
		float f	= blurD.getNumber();
		opts.sblur = f;
		opts.tblur = f;
		opts.rblur = f;
	} else if (blurD.type() == DL::Data::T_Group) {
		DL::DataGroup arr = blurD.getGroup();
		for (uint32 i = 0; i < 3 && i < arr.anonymousCount(); ++i) {
			DL::Data dat = arr.at(i);
			if (dat.isNumber()) {
				float f = dat.getNumber();
				switch (i) {
				case 0:
					opts.sblur = f;
					break;
				case 1:
					opts.tblur = f;
					break;
				case 2:
					opts.rblur = f;
					break;
				}
			}
		}
	}

	if (anisoD.type() == DL::Data::T_Integer) {
		opts.anisotropic = anisoD.getInt();
	}

	std::string filename;
	if (filenameD.type() == DL::Data::T_String) {
		filename = filenameD.getString();
	} else {
		PR_LOG(L_ERROR) << "No valid filename given for texture " << name << std::endl;
		return;
	}

	if (!boost::filesystem::exists(filename) || !boost::filesystem::is_regular_file(filename)) {
		PR_LOG(L_ERROR) << "No valid file fount for texture " << name << " at " << filename << std::endl;
		return;
	}

	std::string type;
	if (typeD.type() == DL::Data::T_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		PR_LOG(L_ERROR) << "No valid type given for texture " << name << std::endl;
		return;
	}

	if (type == "scalar"
		|| type == "grayscale"
		|| type == "color"
		|| type == "spectral") {
		if (env->hasMapSocket(name)) {
			PR_LOG(L_ERROR) << "Texture " << name << " already exists" << std::endl;
			return;
		}

		auto output = std::make_shared<ImageMapSocket>(
			(OIIO::TextureSystem*)env->textureSystem(), opts, filename);
		env->addMapSocket(name, output);
	} else {
		PR_LOG(L_ERROR) << "No known type given for texture " << name << std::endl;
		return;
	}
}
} // namespace PR
