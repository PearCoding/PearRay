#include "OutputSpecification.h"
#include "Logger.h"
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
		auto workDir   = boost::filesystem::canonical(mWorkingDir);
		auto f = workDir / LOCK_FILE_NAME;
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
		auto workDir  = boost::filesystem::canonical(mWorkingDir);
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
	std::shared_ptr<OutputBuffer> output = renderer->output();

	for (File& file : mFiles) {
		for (IM_ChannelSetting3D& cs3d : file.Settings3D) {
			if (!cs3d.LPE_S.empty() || !output->getChannel(cs3d.Variable)) {
				auto ptr = std::make_shared<FrameBufferFloat>(
					3, renderer->width(), renderer->height(), 0.0f);

				if (cs3d.LPE_S.empty())
					output->registerChannel(cs3d.Variable, ptr);
				else
					cs3d.LPE = (int)output->registerLPEChannel(cs3d.Variable, LightPathExpression(cs3d.LPE_S), ptr);
			}
		}

		for (IM_ChannelSetting1D& cs1d : file.Settings1D) {
			if (!cs1d.LPE_S.empty() || !output->getChannel(cs1d.Variable)) {
				auto ptr = std::make_shared<FrameBufferFloat>(
					1, renderer->width(), renderer->height(), 0.0f);

				if (cs1d.LPE_S.empty())
					output->registerChannel(cs1d.Variable, ptr);
				else
					cs1d.LPE = (int)output->registerLPEChannel(cs1d.Variable, LightPathExpression(cs1d.LPE_S), ptr);
			}
		}

		for (IM_ChannelSettingCounter& cs : file.SettingsCounter) {
			if (!cs.LPE_S.empty() || !output->getChannel(cs.Variable)) {
				auto ptr = std::make_shared<FrameBufferUInt32>(
					1, renderer->width(), renderer->height(), 0);

				if (cs.LPE_S.empty())
					output->registerChannel(cs.Variable, ptr);
				else
					cs.LPE = (int)output->registerLPEChannel(cs.Variable, LightPathExpression(cs.LPE_S), ptr);
			}
		}

		for (IM_ChannelSettingSpec& ss : file.SettingsSpectral) {
			auto ptr = std::make_shared<FrameBufferFloat>(
				output->getSpectralChannel()->channels(), renderer->width(), renderer->height(), 0.0f);

			if (!ss.LPE_S.empty())
				ss.LPE = (int)output->registerLPEChannel(LightPathExpression(ss.LPE_S), ptr);
		}
	}
}

static struct {
	const char* Str;
	OutputBuffer::Variable1D Var;
} _s_var1d[] = {
	{ "entity_id", OutputBuffer::V_EntityID },
	{ "entity", OutputBuffer::V_EntityID },
	{ "id", OutputBuffer::V_EntityID },
	{ "material_id", OutputBuffer::V_MaterialID },
	{ "material", OutputBuffer::V_MaterialID },
	{ "mat", OutputBuffer::V_MaterialID },
	{ "emission_id", OutputBuffer::V_EmissionID },
	{ "emission", OutputBuffer::V_EmissionID },
	{ "displace_id", OutputBuffer::V_DisplaceID },
	{ "displace", OutputBuffer::V_DisplaceID },
	{ "depth", OutputBuffer::V_Depth },
	{ "d", OutputBuffer::V_Depth },
	{ "time", OutputBuffer::V_Time },
	{ "t", OutputBuffer::V_Time },
	{ nullptr, OutputBuffer::V_1D_COUNT },
};

static struct {
	const char* Str;
	OutputBuffer::VariableCounter Var;
} _s_varC[] = {
	{ "samples", OutputBuffer::V_Samples },
	{ "s", OutputBuffer::V_Samples },
	{ "feedback", OutputBuffer::V_Feedback },
	{ "f", OutputBuffer::V_Feedback },
	{ "error", OutputBuffer::V_Feedback },
	{ nullptr, OutputBuffer::V_COUNTER_COUNT },
};

static struct {
	const char* Str;
	OutputBuffer::Variable3D Var;
} _s_var3d[] = {
	{ "position", OutputBuffer::V_Position },
	{ "pos", OutputBuffer::V_Position },
	{ "p", OutputBuffer::V_Position },
	{ "normal", OutputBuffer::V_Normal },
	{ "norm", OutputBuffer::V_Normal },
	{ "n", OutputBuffer::V_Normal },
	{ "normal_geometric", OutputBuffer::V_NormalG },
	{ "ng", OutputBuffer::V_NormalG },
	{ "tangent", OutputBuffer::V_Tangent },
	{ "tan", OutputBuffer::V_Tangent },
	{ "nx", OutputBuffer::V_Tangent },
	{ "bitangent", OutputBuffer::V_Bitangent },
	{ "binormal", OutputBuffer::V_Bitangent },
	{ "bi", OutputBuffer::V_Bitangent },
	{ "ny", OutputBuffer::V_Bitangent },
	{ "view", OutputBuffer::V_View },
	{ "v", OutputBuffer::V_View },
	{ "texture", OutputBuffer::V_UVW },
	{ "uvw", OutputBuffer::V_UVW },
	{ "uv", OutputBuffer::V_UVW },
	{ "tex", OutputBuffer::V_UVW },
	{ "velocity", OutputBuffer::V_DPDT },
	{ "movement", OutputBuffer::V_DPDT },
	{ "dpdt", OutputBuffer::V_DPDT },
	{ nullptr, OutputBuffer::V_3D_COUNT },
};

OutputBuffer::Variable1D typeToVariable1D(const std::string& str)
{
	for (size_t i = 0; _s_var1d[i].Str; ++i) {
		if (str == _s_var1d[i].Str)
			return _s_var1d[i].Var;
	}
	return OutputBuffer::V_1D_COUNT; // AS UNKNOWN
}

std::string variableToString(OutputBuffer::Variable1D var)
{
	for (size_t i = 0; _s_var1d[i].Str; ++i) {
		if (var == _s_var1d[i].Var)
			return _s_var1d[i].Str;
	}
	return ""; // AS UNKNOWN
}

OutputBuffer::VariableCounter typeToVariableCounter(const std::string& str)
{
	for (size_t i = 0; _s_varC[i].Str; ++i) {
		if (str == _s_varC[i].Str)
			return _s_varC[i].Var;
	}
	return OutputBuffer::V_COUNTER_COUNT; // AS UNKNOWN
}

std::string variableToString(OutputBuffer::VariableCounter var)
{
	for (size_t i = 0; _s_varC[i].Str; ++i) {
		if (var == _s_varC[i].Var)
			return _s_varC[i].Str;
	}
	return ""; // AS UNKNOWN
}

OutputBuffer::Variable3D typeToVariable3D(const std::string& str)
{
	for (size_t i = 0; _s_var3d[i].Str; ++i) {
		if (str == _s_var3d[i].Str)
			return _s_var3d[i].Var;
	}
	return OutputBuffer::V_3D_COUNT; // AS UNKNOWN
}

std::string variableToString(OutputBuffer::Variable3D var)
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
							OutputBuffer::Variable3D var3D			 = typeToVariable3D(type);
							OutputBuffer::Variable1D var1D			 = typeToVariable1D(type);
							OutputBuffer::VariableCounter varCounter = typeToVariableCounter(type);

							if (var3D != OutputBuffer::V_3D_COUNT) {
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
							} else if (var1D != OutputBuffer::V_1D_COUNT) {
								IM_ChannelSetting1D spec;
								spec.TMM	  = tmm;
								spec.Variable = var1D;
								spec.LPE_S	= lpe;
								spec.LPE	  = -1;
								spec.Name	 = variableToString(var1D);
								if (!lpe.empty())
									spec.Name = spec.Name + "[" + lpe + "]";
								file.Settings1D.push_back(spec);
							} else if (varCounter != OutputBuffer::V_COUNTER_COUNT) {
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

	for (const FileSpectral& f : mSpectralFiles) {
		auto file = outputDir / (f.Name + ".spec");

		auto channel = renderer->output()->getSpectralChannel();
		if (f.LPE >= 0)
			channel = renderer->output()->getSpectralChannel(f.LPE);

		if (!mImageWriter.save_spectral(file.generic_wstring(), channel, f.Compress))
			PR_LOG(L_ERROR) << "Couldn't save spectral file " << file << std::endl;

		if (force)
			PR_LOG(L_INFO) << "Saved file " << file << std::endl;
	}

	if (!force && !boost::filesystem::remove(path / LOCK_IMG_FILE_NAME)) // Remove it now
		PR_LOG(L_ERROR) << "Couldn't delete lock directory " << (path / LOCK_IMG_FILE_NAME) << "!" << std::endl;
}
} // namespace PR
