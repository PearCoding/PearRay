#include "OutputSpecification.h"
#include "Logger.h"
#include "buffer/OutputBuffer.h"
#include "renderer/RenderContext.h"

#include <boost/filesystem.hpp>
#include <sstream>

#include "DataLisp.h"

namespace PR {
using namespace PR;

constexpr static const char* LOCK_FILE_NAME		= "/.pearray_lock";
constexpr static const char* LOCK_IMG_FILE_NAME = "/.pearray_img_lock";

OutputSpecification::OutputSpecification(const std::wstring& wrkDir)
	: mInit(false)
	, mWorkingDir(wrkDir)
{
}

OutputSpecification::~OutputSpecification()
{
	deinit();
}

void OutputSpecification::init(const std::shared_ptr<RenderContext>& context)
{
	if (mInit && !mWorkingDir.empty()) {
		auto workDir = boost::filesystem::canonical(mWorkingDir);
		auto f		 = workDir / LOCK_FILE_NAME;
		if (boost::filesystem::exists(f) && !boost::filesystem::remove(f))
			PR_LOG(L_ERROR) << "Couldn't delete lock directory " << f << std::endl;
	}

	mInit = true;

	if (context && !mWorkingDir.empty()) {
		auto workDir = boost::filesystem::canonical(mWorkingDir);
		mWorkingDir  = workDir.generic_wstring();

		// Setup lock directory
		if (!boost::filesystem::create_directory(workDir / LOCK_FILE_NAME))
			PR_LOG(L_WARNING) << "Couldn't create lock directory " << workDir << LOCK_FILE_NAME << ". Maybe already running?" << std::endl;

		if (!boost::filesystem::remove(workDir / LOCK_IMG_FILE_NAME)) // Remove it now
			PR_LOG(L_ERROR) << "Couldn't delete lock directory " << workDir << LOCK_IMG_FILE_NAME << "!" << std::endl;
	}
}

void OutputSpecification::deinit()
{
	mImageWriter.deinit();
	mFiles.clear();
	mSpectralFiles.clear();

	if (!mInit)
		return;

	if (!mWorkingDir.empty()) {
		auto workDir = boost::filesystem::canonical(mWorkingDir);
		auto f		 = workDir / LOCK_FILE_NAME;
		if (boost::filesystem::exists(f) && !boost::filesystem::remove(f))
			PR_LOG(L_ERROR) << "Couldn't delete lock directory " << f;
	}

	mInit = false;
}

void OutputSpecification::setup(const std::shared_ptr<RenderContext>& renderer)
{
	PR_ASSERT(renderer, "Given renderer has to be valid");

	if (!isInit())
		init(renderer);

	mImageWriter.init(renderer);
	OutputBufferData& data = renderer->output()->data();

	for (File& file : mFiles) {
		for (IM_ChannelSetting3D& cs3d : file.Settings3D) {
			if (!cs3d.LPE_S.empty() || !data.hasInternalChannel_3D(cs3d.Variable)) {
				if (cs3d.LPE_S.empty())
					data.requestInternalChannel_3D(cs3d.Variable);
				else {
					size_t id;
					data.requestLPEChannel_3D(cs3d.Variable, LightPathExpression(cs3d.LPE_S), id);
					cs3d.LPE = (int)id;
				}
			}
		}

		for (IM_ChannelSetting1D& cs1d : file.Settings1D) {
			if (!cs1d.LPE_S.empty() || !data.hasInternalChannel_1D(cs1d.Variable)) {
				if (cs1d.LPE_S.empty())
					data.requestInternalChannel_1D(cs1d.Variable);
				else {
					size_t id;
					data.requestLPEChannel_1D(cs1d.Variable, LightPathExpression(cs1d.LPE_S), id);
					cs1d.LPE = (int)id;
				}
			}
		}

		for (IM_ChannelSettingCounter& cs : file.SettingsCounter) {
			if (!cs.LPE_S.empty() || !data.hasInternalChannel_Counter(cs.Variable)) {
				if (cs.LPE_S.empty())
					data.requestInternalChannel_Counter(cs.Variable);
				else {
					size_t id;
					data.requestLPEChannel_Counter(cs.Variable, LightPathExpression(cs.LPE_S), id);
					cs.LPE = (int)id;
				}
			}
		}

		for (IM_ChannelSettingSpec& ss : file.SettingsSpectral) {
			if (!ss.LPE_S.empty()) {
				size_t id;
				data.requestLPEChannel_Spectral(LightPathExpression(ss.LPE_S), id);
				ss.LPE = (int)id;
			}
		}
	}
}

static struct {
	const char* Str;
	AOV1D Var;
} _s_var1d[] = {
	{ "entity_id", AOV_EntityID },
	{ "entity", AOV_EntityID },
	{ "id", AOV_EntityID },
	{ "material_id", AOV_MaterialID },
	{ "material", AOV_MaterialID },
	{ "mat", AOV_MaterialID },
	{ "emission_id", AOV_EmissionID },
	{ "emission", AOV_EmissionID },
	{ "displace_id", AOV_DisplaceID },
	{ "displace", AOV_DisplaceID },
	{ "depth", AOV_Depth },
	{ "d", AOV_Depth },
	{ "time", AOV_Time },
	{ "t", AOV_Time },
	{ nullptr, AOV_1D_COUNT },
};

static struct {
	const char* Str;
	AOVCounter Var;
} _s_varC[] = {
	{ "sample_count", AOV_SampleCount },
	{ "samples", AOV_SampleCount },
	{ "s", AOV_SampleCount },
	{ "feedback", AOV_Feedback },
	{ "f", AOV_Feedback },
	{ "error", AOV_Feedback },
	{ nullptr, AOV_COUNTER_COUNT },
};

static struct {
	const char* Str;
	AOV3D Var;
} _s_var3d[] = {
	{ "position", AOV_Position },
	{ "pos", AOV_Position },
	{ "p", AOV_Position },
	{ "normal", AOV_Normal },
	{ "norm", AOV_Normal },
	{ "n", AOV_Normal },
	{ "normal_geometric", AOV_NormalG },
	{ "ng", AOV_NormalG },
	{ "tangent", AOV_Tangent },
	{ "tan", AOV_Tangent },
	{ "nx", AOV_Tangent },
	{ "bitangent", AOV_Bitangent },
	{ "binormal", AOV_Bitangent },
	{ "bi", AOV_Bitangent },
	{ "ny", AOV_Bitangent },
	{ "view", AOV_View },
	{ "v", AOV_View },
	{ "texture", AOV_UVW },
	{ "uvw", AOV_UVW },
	{ "uv", AOV_UVW },
	{ "tex", AOV_UVW },
	{ "velocity", AOV_DPDT },
	{ "movement", AOV_DPDT },
	{ "dpdt", AOV_DPDT },
	{ nullptr, AOV_3D_COUNT },
};

AOV1D typeToVariable1D(const std::string& str)
{
	for (size_t i = 0; _s_var1d[i].Str; ++i) {
		if (str == _s_var1d[i].Str)
			return _s_var1d[i].Var;
	}
	return AOV_1D_COUNT; // AS UNKNOWN
}

std::string variableToString(AOV1D var)
{
	for (size_t i = 0; _s_var1d[i].Str; ++i) {
		if (var == _s_var1d[i].Var)
			return _s_var1d[i].Str;
	}
	return ""; // AS UNKNOWN
}

AOVCounter typeToVariableCounter(const std::string& str)
{
	for (size_t i = 0; _s_varC[i].Str; ++i) {
		if (str == _s_varC[i].Str)
			return _s_varC[i].Var;
	}
	return AOV_COUNTER_COUNT; // AS UNKNOWN
}

std::string variableToString(AOVCounter var)
{
	for (size_t i = 0; _s_varC[i].Str; ++i) {
		if (var == _s_varC[i].Var)
			return _s_varC[i].Str;
	}
	return ""; // AS UNKNOWN
}

AOV3D typeToVariable3D(const std::string& str)
{
	for (size_t i = 0; _s_var3d[i].Str; ++i) {
		if (str == _s_var3d[i].Str)
			return _s_var3d[i].Var;
	}
	return AOV_3D_COUNT; // AS UNKNOWN
}

std::string variableToString(AOV3D var)
{
	for (size_t i = 0; _s_var3d[i].Str; ++i) {
		if (var == _s_var3d[i].Var)
			return _s_var3d[i].Str;
	}
	return "UNKNOWN";
}

void OutputSpecification::parse(Environment*, const DL::DataGroup& group)
{
	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		DL::Data dataD = group.at(i);

		if (dataD.type() == DL::DT_Group) {
			DL::DataGroup entry = dataD.getGroup();

			if (entry.id() == "output") {
				DL::Data nameD = entry.getFromKey("name");
				if (nameD.type() != DL::DT_String)
					continue;

				File file;
				file.Name = nameD.getString();

				for (size_t i = 0; i < entry.anonymousCount(); ++i) {
					DL::Data channelD = entry.at(i);

					if (channelD.type() == DL::DT_Group
						&& channelD.getGroup().id() == "channel") {
						DL::DataGroup channel = channelD.getGroup();
						DL::Data typeD		  = channel.getFromKey("type");
						DL::Data colorD		  = channel.getFromKey("color");
						DL::Data gammaD		  = channel.getFromKey("gamma");
						DL::Data mapperD	  = channel.getFromKey("mapper");
						DL::Data lpeD		  = channel.getFromKey("lpe");

						if (typeD.type() != DL::DT_String)
							continue;

						std::string type = typeD.getString();
						std::transform(type.begin(), type.end(), type.begin(), ::tolower);

						ToneColorMode tcm = TCM_SRGB;
						if (colorD.type() == DL::DT_String) {
							std::string color = colorD.getString();
							std::transform(color.begin(), color.end(), color.begin(), ::tolower);
							if (color == "xyz")
								tcm = TCM_XYZ;
							else if (color == "norm_xyz")
								tcm = TCM_XYZ_NORM;
							else if (color == "lum" || color == "luminance" || color == "gray")
								tcm = TCM_LUMINANCE;
						}

						ToneGammaMode tgm = TGM_SRGB;
						if (gammaD.type() == DL::DT_String) {
							std::string gamma = gammaD.getString();
							std::transform(gamma.begin(), gamma.end(), gamma.begin(), ::tolower);
							if (gamma == "none")
								tgm = TGM_None;
						}

						ToneMapperMode tmm = TMM_None;
						if (mapperD.type() == DL::DT_String) {
							std::string mapper = mapperD.getString();
							std::transform(mapper.begin(), mapper.end(), mapper.begin(), ::tolower);
							if (mapper == "reinhard")
								tmm = TMM_Simple_Reinhard;
							else if (mapper == "clamp")
								tmm = TMM_Clamp;
							else if (mapper == "absolute" || mapper == "abs")
								tmm = TMM_Abs;
							else if (mapper == "positive" || mapper == "pos")
								tmm = TMM_Positive;
							else if (mapper == "negative" || mapper == "neg")
								tmm = TMM_Negative;
							else if (mapper == "spherical")
								tmm = TMM_Spherical;
							else if (mapper == "normalize" || mapper == "normalized" || mapper == "norm")
								tmm = TMM_Normalized;
						}

						std::string lpe = "";
						if (lpeD.type() == DL::DT_String) {
							lpe = lpeD.getString();
							if (!lpe.empty() && !LightPathExpression(lpe).isValid()) {
								PR_LOG(L_ERROR) << "Invalid LPE " << lpe << ". Skipping entry" << std::endl;
								lpe = "";
							}
						}

						if (type == "rgb" || type == "color") {
							IM_ChannelSettingSpec spec;
							spec.Elements = 0; //TODO
							spec.TCM	  = tcm;
							spec.TGM	  = tgm;
							spec.TMM	  = tmm;
							spec.LPE_S	= lpe;
							spec.LPE	  = -1;

							if (!lpe.empty())
								spec.Name = spec.Name + "[" + lpe + "]";
							file.SettingsSpectral.push_back(spec);
						} else {
							AOV3D var3D			  = typeToVariable3D(type);
							AOV1D var1D			  = typeToVariable1D(type);
							AOVCounter varCounter = typeToVariableCounter(type);

							if (var3D != AOV_3D_COUNT) {
								IM_ChannelSetting3D spec;
								spec.Elements = 0; //TODO
								spec.TMM	  = tmm;
								spec.Variable = var3D;
								spec.LPE_S	= lpe;
								spec.LPE	  = -1;

								std::string name = variableToString(var3D);
								if (!lpe.empty())
									name = name + "[" + lpe + "]";
								spec.Name[0] = name + ".x";
								spec.Name[1] = name + ".y";
								spec.Name[2] = name + ".z";

								file.Settings3D.push_back(spec);
							} else if (var1D != AOV_1D_COUNT) {
								IM_ChannelSetting1D spec;
								spec.TMM	  = tmm;
								spec.Variable = var1D;
								spec.LPE_S	= lpe;
								spec.LPE	  = -1;
								spec.Name	 = variableToString(var1D);
								if (!lpe.empty())
									spec.Name = spec.Name + "[" + lpe + "]";
								file.Settings1D.push_back(spec);
							} else if (varCounter != AOV_COUNTER_COUNT) {
								IM_ChannelSettingCounter spec;
								spec.TMM	  = tmm;
								spec.Variable = varCounter;
								spec.LPE_S	= lpe;
								spec.LPE	  = -1;
								spec.Name	 = variableToString(varCounter);
								if (!lpe.empty())
									spec.Name = spec.Name + "[" + lpe + "]";
								file.SettingsCounter.push_back(spec);
							} else {
								PR_LOG(L_ERROR) << "Unknown channel type " << type;
							}
						}
					}
				}

				mFiles.push_back(file);
			} else if (entry.id() == "output_spectral") {
				DL::Data nameD	 = entry.getFromKey("name");
				DL::Data compressD = entry.getFromKey("compress");
				DL::Data lpeD	  = entry.getFromKey("lpe");
				if (nameD.type() == DL::DT_String) {
					FileSpectral spec;
					spec.Name	 = nameD.getString();
					spec.Compress = compressD.type() == DL::DT_Bool ? compressD.getBool() : true;
					spec.LPE_S	= "";
					spec.LPE	  = -1;

					if (lpeD.type() == DL::DT_String) {
						spec.LPE_S = lpeD.getString();
						if (!spec.LPE_S.empty() && !LightPathExpression(spec.LPE_S).isValid()) {
							PR_LOG(L_ERROR) << "Invalid LPE " << spec.LPE_S << ". Skipping spectral file" << std::endl;
							spec.LPE_S = "";
						}
					}
					mSpectralFiles.push_back(spec);
				}
			}
		}
	}
}

void OutputSpecification::save(const std::shared_ptr<RenderContext>& renderer,
							   ToneMapper& toneMapper, bool force) const
{
	boost::filesystem::path path = mWorkingDir;

	if (!force && !boost::filesystem::create_directory(path / LOCK_IMG_FILE_NAME))
		return;

	std::wstring resultDir = L"/results";
	if (renderer->index() > 0) {
		std::wstringstream stream;
		stream << resultDir << L"_" << renderer->index();
		resultDir = stream.str();
	}

	const auto outputDir = path / resultDir;
	boost::filesystem::create_directory(outputDir); // Doesn't matter if it works or not

	for (const File& f : mFiles) {
		auto file = outputDir / (f.Name + ".exr");
		if (!mImageWriter.save(toneMapper, file.generic_wstring(),
							   f.SettingsSpectral, f.Settings1D, f.SettingsCounter, f.Settings3D))
			PR_LOG(L_ERROR) << "Couldn't save image file " << file << std::endl;

		if (force)
			PR_LOG(L_INFO) << "Saved file " << file << std::endl;
	}

	const OutputBufferData& data = renderer->output()->data();
	for (const FileSpectral& f : mSpectralFiles) {
		auto file = outputDir / (f.Name + ".spec");

		auto channel = data.getInternalChannel_Spectral();
		if (f.LPE >= 0)
			channel = data.getLPEChannel_Spectral(f.LPE);

		if (!mImageWriter.save_spectral(file.generic_wstring(), channel, f.Compress))
			PR_LOG(L_ERROR) << "Couldn't save spectral file " << file << std::endl;

		if (force)
			PR_LOG(L_INFO) << "Saved file " << file << std::endl;
	}

	if (!force && !boost::filesystem::remove(path / LOCK_IMG_FILE_NAME)) // Remove it now
		PR_LOG(L_ERROR) << "Couldn't delete lock directory " << (path / LOCK_IMG_FILE_NAME) << "!" << std::endl;
}
} // namespace PR
