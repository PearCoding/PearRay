#include "TextureParser.h"
#include "Environment.h"
#include "ImageIO.h"
#include "Logger.h"
#include "SceneLoader.h"
#include "spectral/SpectralUpsampler.h"

#include "shader/ImageNode.h"

#include "DataLisp.h"

#include <algorithm>
#include <filesystem>

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

void TextureParser::parse(SceneLoadContext& ctx, const std::string& name, const DL::DataGroup& group)
{
	DL::Data filenameD = group.getFromKey("file");
	if (!filenameD.isValid())
		filenameD = group.getFromKey("filename");

	DL::Data typeD = group.getFromKey("type");
	//DL::Data dimD = group.getFromKey("dimension");
	DL::Data wrapModeD			= group.getFromKey("wrap");
	DL::Data mipModeD			= group.getFromKey("mip");
	DL::Data interpolationModeD = group.getFromKey("interpolation");
	DL::Data blurD				= group.getFromKey("blur");
	DL::Data anisoD				= group.getFromKey("anisotropic");
	DL::Data parametricD		= group.getFromKey("parametric");

	OIIO::TextureOpt opts;
	if (wrapModeD.type() == DL::DT_String) {
		std::string wrap = wrapModeD.getString();
		std::transform(wrap.begin(), wrap.end(), wrap.begin(), ::tolower);
		opts.swrap = parseWrap(wrap);
		opts.twrap = parseWrap(wrap);
		opts.rwrap = parseWrap(wrap);
	} else if (wrapModeD.type() == DL::DT_Group) {
		DL::DataGroup arr = wrapModeD.getGroup();
		for (uint32 i = 0; i < 3 && i < arr.anonymousCount(); ++i) {
			DL::Data dat = arr.at(i);
			if (dat.type() == DL::DT_String) {
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

	if (mipModeD.type() == DL::DT_String) {
		std::string mip = mipModeD.getString();
		std::transform(mip.begin(), mip.end(), mip.begin(), ::tolower);
		opts.mipmode = parseMIP(mip);
	} else {
		opts.mipmode = OIIO::TextureOpt::MipModeNoMIP;// For now
	}

	if (interpolationModeD.type() == DL::DT_String) {
		std::string interp = interpolationModeD.getString();
		std::transform(interp.begin(), interp.end(), interp.begin(), ::tolower);
		opts.interpmode = parseInterpolation(interp);
	}

	if (blurD.isNumber()) {
		float f	   = blurD.getNumber();
		opts.sblur = f;
		opts.tblur = f;
		opts.rblur = f;
	} else if (blurD.type() == DL::DT_Group) {
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

	if (anisoD.type() == DL::DT_Integer) {
		opts.anisotropic = static_cast<int>(anisoD.getInt());
	}

	std::filesystem::path filename;
	if (filenameD.type() == DL::DT_String) {
		if (parametricD.type() == DL::DT_Bool && parametricD.getBool())
			filename = ctx.escapePath(filenameD.getString());
		else
			filename = ctx.setupParametricImage(filenameD.getString());
	} else {
		PR_LOG(L_ERROR) << "No valid filename given for texture " << name << std::endl;
		return;
	}

	if (!std::filesystem::exists(filename) || !std::filesystem::is_regular_file(filename)) {
		PR_LOG(L_ERROR) << "No valid file found for texture " << name << " at " << filename << std::endl;
		return;
	}

	std::string type;
	if (typeD.type() == DL::DT_String) {
		type = typeD.getString();
		std::transform(type.begin(), type.end(), type.begin(), ::tolower);
	} else {
		PR_LOG(L_WARNING) << "No valid type given for texture " << name << ": Assuming color" << std::endl;
		type = "color";
	}

	if (type == "scalar"
		|| type == "grayscale"
		|| type == "color"
		|| type == "spectral") {
		if (ctx.environment()->hasNode(name)) {
			PR_LOG(L_ERROR) << "Texture " << name << " already exists" << std::endl;
			return;
		}

		auto output = std::make_shared<ImageNode>(
			(OIIO::TextureSystem*)ctx.environment()->textureSystem(),
			opts, filename);
		ctx.environment()->addNode(name, output);
	} else {
		PR_LOG(L_ERROR) << "No known type given for texture " << name << std::endl;
		return;
	}
}

void TextureParser::convertToParametric(const SceneLoadContext& ctx, const std::filesystem::path& input, const std::filesystem::path& output)
{
	const auto upsampler = ctx.environment()->defaultSpectralUpsampler();

	size_t width, height, channels;
	std::vector<float> data;
	if (!ImageIO::load(input, data, width, height, channels)) {
		PR_LOG(L_ERROR) << "Can not load " << input << std::endl;
		return;
	}

	if (channels == 1) {
		// Convert grayscale to RGB
		std::vector<float> new_data;
		new_data.resize(width * height * 3);
		for (size_t i = 0; i < width * height; ++i) {
			float g				= data[i];
			new_data[i * 3 + 0] = g;
			new_data[i * 3 + 1] = g;
			new_data[i * 3 + 2] = g;
		}
		data = std::move(new_data);
	} else if (channels == 4) {
		// Convert RGBA to RGB
		std::vector<float> new_data;
		new_data.resize(width * height * 3);
		for (size_t i = 0; i < width * height; ++i) {
			new_data[i * 3 + 0] = data[i * 4 + 0];
			new_data[i * 3 + 1] = data[i * 4 + 1];
			new_data[i * 3 + 2] = data[i * 4 + 2];
		}
		data = std::move(new_data);
	} else if (channels != 3) {
		PR_LOG(L_ERROR) << "Can not convert " << input << " to parametric due to invalid channel count" << std::endl;
		return;
	}

	std::vector<float> output_data;
	output_data.resize(data.size());
	upsampler->prepare(data.data(), output_data.data(), width * height);

	ImageSaveOptions opts;
	opts.Parametric = true;
	if (!ImageIO::save(output, output_data.data(), width, height, 3, opts)) {
		PR_LOG(L_ERROR) << "Can not write image " << output << std::endl;
		return;
	}
}
} // namespace PR
