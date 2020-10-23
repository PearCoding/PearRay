#include "OutputSpecification.h"
#include "Logger.h"
#include "arch/FileLock.h"
#include "buffer/FrameBufferSystem.h"
#include "renderer/RenderContext.h"

#include <filesystem>
#include <sstream>

#include "DataLisp.h"

namespace PR {
constexpr static const wchar_t* LOCK_RUN_FILE_NAME = L".pearray_run.lock";
constexpr static const wchar_t* LOCK_OUT_FILE_NAME = L".pearray_out.lock";

OutputSpecification::OutputSpecification(const std::filesystem::path& wrkDir)
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
	std::wstring runlock	= LOCK_RUN_FILE_NAME;
	std::wstring outputlock = LOCK_OUT_FILE_NAME;

	if (context && !mWorkingDir.empty()) {
		auto workDir = std::filesystem::canonical(mWorkingDir);
		mWorkingDir	 = workDir.generic_wstring();

		runlock	   = (workDir / runlock).generic_wstring();
		outputlock = (workDir / outputlock).generic_wstring();
	}

	mRunLock	= std::make_unique<FileLock>(runlock);
	mOutputLock = std::make_unique<FileLock>(outputlock);
	mInit		= true;

	if (!mRunLock->lock())
		PR_LOG(L_ERROR) << "Couldn't create lock " << std::filesystem::path(runlock) << std::endl;
}

void OutputSpecification::deinit()
{
	mImageWriter.deinit();
	mFiles.clear();

	mRunLock.reset();
	mOutputLock.reset();

	mInit = false;
}

void OutputSpecification::setup(const std::shared_ptr<RenderContext>& renderer)
{
	PR_ASSERT(renderer, "Given renderer has to be valid");

	if (!isInit())
		init(renderer);

	mImageWriter.init(renderer);
	FrameBufferContainer& data = renderer->output()->data();

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
			if (!ss.LPE_S.empty() || !data.hasInternalChannel_Spectral(ss.Variable)) {
				if (ss.LPE_S.empty())
					data.requestInternalChannel_Spectral(ss.Variable);
				else {
					size_t id;
					data.requestLPEChannel_Spectral(ss.Variable, LightPathExpression(ss.LPE_S), id);
					ss.LPE = (int)id;
				}
			}
		}
	}
}

static struct {
	const char* Str;
	AOVSpectral Var;
} _s_varSpectral[] = {
	{ "color", AOV_Output },
	{ "spectral", AOV_Output },
	{ "output", AOV_Output },
	{ "rgb", AOV_Output },
	{ "online_mean", AOV_OnlineMean },
	{ "variance", AOV_OnlineVariance },
	{ "online_variance", AOV_OnlineVariance },
	{ "var", AOV_OnlineVariance },
	{ nullptr, AOV_SPECTRAL_COUNT },
};

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
	{ "pixel", AOV_PixelWeight },
	{ "blend", AOV_PixelWeight },
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
	{ nullptr, AOV_3D_COUNT },
};

AOVSpectral typeToVariableSpectral(const std::string& str)
{
	for (size_t i = 0; _s_varSpectral[i].Str; ++i) {
		if (str == _s_varSpectral[i].Str)
			return _s_varSpectral[i].Var;
	}
	return AOV_SPECTRAL_COUNT; // AS UNKNOWN
}

std::string variableToString(AOVSpectral var)
{
	for (size_t i = 0; _s_varSpectral[i].Str; ++i) {
		if (var == _s_varSpectral[i].Var)
			return _s_varSpectral[i].Str;
	}
	return ""; // AS UNKNOWN
}

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

void OutputSpecification::parse(Environment*, const DL::DataGroup& entry)
{
	PR_ASSERT(entry.id() == "output", "Can only be of tag 'output'");
	DL::Data nameD = entry.getFromKey("name");
	if (nameD.type() != DL::DT_String) {
		PR_LOG(L_ERROR) << "No name given for output" << std::endl;
		return;
	}

	File file;
	file.Name = nameD.getString();

	for (size_t i = 0; i < entry.anonymousCount(); ++i) {
		DL::Data channelD = entry.at(i);

		if (channelD.type() == DL::DT_Group
			&& channelD.getGroup().id() == "channel") {
			DL::DataGroup channel = channelD.getGroup();
			DL::Data typeD		  = channel.getFromKey("type");
			DL::Data colorD		  = channel.getFromKey("color");
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

			std::string lpe = "";
			if (lpeD.type() == DL::DT_String) {
				lpe = lpeD.getString();
				if (!lpe.empty() && !LightPathExpression(lpe).isValid()) {
					PR_LOG(L_ERROR) << "Invalid LPE '" << lpe << "'. Skipping entry" << std::endl;
					lpe = "";
				}
			}

			const AOVSpectral varSpectral = typeToVariableSpectral(type);
			const AOV3D var3D			  = typeToVariable3D(type);
			const AOV1D var1D			  = typeToVariable1D(type);
			const AOVCounter varCounter	  = typeToVariableCounter(type);

			if (varSpectral != AOV_SPECTRAL_COUNT) {
				IM_ChannelSettingSpec spec;
				spec.Variable = varSpectral;
				spec.TCM	  = tcm;
				spec.LPE_S	  = lpe;
				spec.LPE	  = -1;

				if (varSpectral != AOV_Output) {
					spec.IsRaw = true;
					spec.Name  = variableToString(varSpectral);
				}
				// else keep it empty

				if (!lpe.empty())
					spec.Name = spec.Name + "[" + lpe + "]";
				file.SettingsSpectral.push_back(spec);
			} else if (var3D != AOV_3D_COUNT) {
				IM_ChannelSetting3D spec;
				spec.Variable = var3D;
				spec.LPE_S	  = lpe;
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
				spec.Variable = var1D;
				spec.LPE_S	  = lpe;
				spec.LPE	  = -1;
				spec.Name	  = variableToString(var1D);
				if (!lpe.empty())
					spec.Name = spec.Name + "[" + lpe + "]";
				file.Settings1D.push_back(spec);
			} else if (varCounter != AOV_COUNTER_COUNT) {
				IM_ChannelSettingCounter spec;
				spec.Variable = varCounter;
				spec.LPE_S	  = lpe;
				spec.LPE	  = -1;
				spec.Name	  = variableToString(varCounter);
				if (!lpe.empty())
					spec.Name = spec.Name + "[" + lpe + "]";
				file.SettingsCounter.push_back(spec);
			} else {
				PR_LOG(L_ERROR) << "Unknown channel type " << type;
			}
		}
	}

	mFiles.push_back(file);
}

void OutputSpecification::save(RenderContext* renderer,
							   ToneMapper& toneMapper, const OutputSaveOptions& options) const
{
	std::filesystem::path path = mWorkingDir;

	if (!options.Force && !mOutputLock->lock())
		return;

	std::wstring resultDir = L"results";
	if (renderer->index() > 0) {
		std::wstringstream stream;
		stream << resultDir << L"_" << renderer->index();
		resultDir = stream.str();
	}

	const auto outputDir = path / resultDir;
	std::filesystem::create_directory(outputDir); // Doesn't matter if it works or not

	for (const File& f : mFiles) {
		auto file = outputDir / (f.Name + options.NameSuffix + ".exr");
		if (!mImageWriter.save(toneMapper, file.generic_wstring(),
							   f.SettingsSpectral, f.Settings1D, f.SettingsCounter, f.Settings3D,
							   options.Image))
			PR_LOG(L_ERROR) << "Couldn't save image file " << file << std::endl;

		if (options.Force)
			PR_LOG(L_INFO) << "Saved file " << file << std::endl;
	}

	mOutputLock->unlock();
}
} // namespace PR
