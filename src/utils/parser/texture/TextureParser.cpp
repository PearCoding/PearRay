#include "TextureParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "shader/ImageScalarOutput.h"
#include "shader/ImageSpectralOutput.h"
#include "shader/ImageVectorOutput.h"

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "DataArray.h"
#include "Data.h"
#include "SourceLogger.h"

#include <algorithm>

using namespace PR;
namespace PRU
{
	OIIO::TextureOpt::Wrap parseWrap(const std::string& name)
	{
		if(name == "black")
			return OIIO::TextureOpt::WrapBlack;
		else if(name == "clamp")
			return OIIO::TextureOpt::WrapClamp;
		else if(name == "periodic")
			return OIIO::TextureOpt::WrapPeriodic;
		else if(name == "mirror")
			return OIIO::TextureOpt::WrapMirror;
		else
			return OIIO::TextureOpt::WrapDefault;
	}

	OIIO::TextureOpt::MipMode parseMIP(const std::string& name)
	{
		if(name == "none")
			return OIIO::TextureOpt::MipModeNoMIP;
		else if(name == "one" || name == "linear")
			return OIIO::TextureOpt::MipModeOneLevel;
		else if(name == "trilinear" || name == "tri")
			return OIIO::TextureOpt::MipModeTrilinear;
		else if(name == "anisotropic" || name == "aniso")
			return OIIO::TextureOpt::MipModeAniso;
		else
			return OIIO::TextureOpt::MipModeDefault;
	}

	OIIO::TextureOpt::InterpMode parseInterpolation(const std::string& name)
	{
		if(name == "closest")
			return OIIO::TextureOpt::InterpClosest;
		else if(name == "bi" || name == "bilinear")
			return OIIO::TextureOpt::InterpBilinear;
		else if(name == "bicubic")
			return OIIO::TextureOpt::InterpBicubic;
		else
			return OIIO::TextureOpt::InterpSmartBicubic;
	}

	void TextureParser::parse(SceneLoader* loader, Environment* env, const std::string& name, DL::DataGroup* group) const
	{
		DL::Data* filenameD = group->getFromKey("file");
		DL::Data* typeD = group->getFromKey("type");
		//DL::Data* dimD = group->getFromKey("dimension");
		DL::Data* wrapModeD = group->getFromKey("wrap");
		DL::Data* mipModeD = group->getFromKey("mip");
		DL::Data* interpolationModeD = group->getFromKey("interpolation");
		DL::Data* blurD = group->getFromKey("blur");
		DL::Data* anisoD = group->getFromKey("anisotropic");

		OIIO::TextureOpt opts;
		if(wrapModeD && wrapModeD->isType() == DL::Data::T_String)
		{
			std::string wrap = wrapModeD->getString();
			std::transform(wrap.begin(), wrap.end(), wrap.begin(), ::tolower);
			opts.swrap = parseWrap(wrap);
			opts.twrap = parseWrap(wrap);
			opts.rwrap = parseWrap(wrap);
		}
		else if(wrapModeD && wrapModeD->isType() == DL::Data::T_Array)
		{
			DL::DataArray* arr = wrapModeD->getArray();
			for(uint32 i = 0; i < 3 && i < arr->size(); ++i)
			{
				DL::Data* dat = arr->at(i);
				if(dat->isType() == DL::Data::T_String)
				{
					std::string wrap = dat->getString();
					std::transform(wrap.begin(), wrap.end(), wrap.begin(), ::tolower);
					switch(i)
					{
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

		if(mipModeD && mipModeD->isType() == DL::Data::T_String)
		{
			std::string mip = mipModeD->getString();
			std::transform(mip.begin(), mip.end(), mip.begin(), ::tolower);
			opts.mipmode = parseMIP(mip);
		}

		if(interpolationModeD && interpolationModeD->isType() == DL::Data::T_String)
		{
			std::string interp = interpolationModeD->getString();
			std::transform(interp.begin(), interp.end(), interp.begin(), ::tolower);
			opts.interpmode = parseInterpolation(interp);
		}

		if(blurD && blurD->isNumber())
		{
			float f = blurD->getFloatConverted();
			opts.sblur = f;
			opts.tblur = f;
			opts.rblur = f;
		}
		else if(blurD && blurD->isType() == DL::Data::T_Array)
		{
			DL::DataArray* arr = blurD->getArray();
			for(uint32 i = 0; i < 3 && i < arr->size(); ++i)
			{
				DL::Data* dat = arr->at(i);
				if(dat->isNumber())
				{
					float f = dat->getFloatConverted();
					switch(i)
					{
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

		if(anisoD && anisoD->isType() == DL::Data::T_Integer)
		{
			opts.anisotropic = anisoD->getInt();
		}

		std::string filename;
		if(filenameD && filenameD->isType() == DL::Data::T_String)
		{
			filename = filenameD->getString();
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No valid filename given for texture '%s'", name.c_str());
			return;
		}

		std::string type;
		if(typeD && typeD->isType() == DL::Data::T_String)
		{
			type = typeD->getString();
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No valid type given for texture '%s'", name.c_str());
			return;
		}

		if(type == "scalar" || type == "grayscale")
		{
			if(env->hasScalarShaderOutput(name))
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Texture '%s' already exists", name.c_str());
				return;
			}

			ImageScalarShaderOutput* output = new ImageScalarShaderOutput(env->textureSystem(), opts, filename);
			env->addShaderOutput(name, output);
		}
		else if(type == "color" || type == "spectral")
		{
			if(env->hasSpectralShaderOutput(name))
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Texture '%s' already exists", name.c_str());
				return;
			}

			ImageSpectralShaderOutput* output = new ImageSpectralShaderOutput(env->textureSystem(), opts, filename);
			env->addShaderOutput(name, output);
		}
		else if(type == "vector")
		{
			if(env->hasVectorShaderOutput(name))
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Texture '%s' already exists", name.c_str());
				return;
			}

			ImageVectorShaderOutput* output = new ImageVectorShaderOutput(env->textureSystem(), opts, filename);
			env->addShaderOutput(name, output);
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No known type given for texture '%s'", name.c_str());
			return;
		}
	}
}