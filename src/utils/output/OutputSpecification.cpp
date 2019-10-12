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

OutputSpecification::OutputSpecification(const std::string& wrkDir)
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
		std::string f = mWorkingDir + LOCK_FILE_NAME;
		if (boost::filesystem::exists(f) && !boost::filesystem::remove(f))
			PR_LOG(L_ERROR) << "Couldn't delete lock directory " << f << std::endl;
	}

	mInit		= true;

	if (context && !mWorkingDir.empty()) {
		mWorkingDir = boost::filesystem::canonical(mWorkingDir).string();

		// Setup lock directory
		if (!boost::filesystem::create_directory(mWorkingDir + LOCK_FILE_NAME))
			PR_LOG(L_WARNING) << "Couldn't create lock directory " << mWorkingDir << LOCK_FILE_NAME << ". Maybe already running?" << std::endl;

		if (!boost::filesystem::remove(mWorkingDir + LOCK_IMG_FILE_NAME)) // Remove it now
			PR_LOG(L_ERROR) << "Couldn't delete lock directory " << mWorkingDir << LOCK_IMG_FILE_NAME << "!" << std::endl;
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
		std::string f = mWorkingDir + LOCK_FILE_NAME;
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

	for (const File& file : mFiles) {
		for (const IM_ChannelSetting3D& cs3d : file.Settings3D) {
			if (!output->getChannel(cs3d.Variable))
				output->registerChannel(cs3d.Variable,
										std::make_shared<FrameBufferFloat>(
											3, renderer->width(), renderer->height(), 0.0f));
		}

		for (const IM_ChannelSetting1D& cs1d : file.Settings1D) {
			if (!output->getChannel(cs1d.Variable))
				output->registerChannel(cs1d.Variable,
										std::make_shared<FrameBufferFloat>(
											1, renderer->width(), renderer->height(), 0));
		}

		for (const IM_ChannelSettingCounter& cs : file.SettingsCounter) {
			if (!output->getChannel(cs.Variable))
				output->registerChannel(cs.Variable,
										std::make_shared<FrameBufferUInt32>(
											1, renderer->width(), renderer->height(), 0));
		}
	}
}

OutputBuffer::Variable1D typeToVariable1D(const std::string& str)
{
	if (str == "depth" || str == "d")
		return OutputBuffer::V_Depth;
	else if (str == "time" || str == "t")
		return OutputBuffer::V_Time;
	else if (str == "material" || str == "mat" || str == "m")
		return OutputBuffer::V_Material;
	else
		return OutputBuffer::V_1D_COUNT; // AS UNKNOWN
}

OutputBuffer::VariableCounter typeToVariableCounter(const std::string& str)
{
	if (str == "id")
		return OutputBuffer::V_ID;
	else if (str == "samples" || str == "s")
		return OutputBuffer::V_Samples;
	else
		return OutputBuffer::V_COUNTER_COUNT; // AS UNKNOWN
}

OutputBuffer::Variable3D typeToVariable3D(const std::string& str)
{
	if (str == "position" || str == "pos" || str == "p")
		return OutputBuffer::V_Position;
	else if (str == "normal" || str == "norm" || str == "n")
		return OutputBuffer::V_Normal;
	else if (str == "geometric_normal" || str == "ng")
		return OutputBuffer::V_NormalG;
	else if (str == "tangent" || str == "tan" || str == "nx")
		return OutputBuffer::V_Tangent;
	else if (str == "bitangent" || str == "binormal" || str == "bi" || str == "ny")
		return OutputBuffer::V_Bitangent;
	else if (str == "view" || str == "v")
		return OutputBuffer::V_View;
	else if (str == "uv" || str == "texture" || str == "uvw" || str == "tex")
		return OutputBuffer::V_UVW;
	else if (str == "velocity" || str == "movement" || str == "dpdt")
		return OutputBuffer::V_DPDT;
	else
		return OutputBuffer::V_3D_COUNT; // AS UNKNOWN
}

void OutputSpecification::parse(Environment*, const DL::DataGroup& group)
{
	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		DL::Data dataD = group.at(i);

		if (dataD.type() == DL::Data::T_Group) {
			DL::DataGroup entry = dataD.getGroup();

			if (entry.id() == "output") {
				DL::Data nameD = entry.getFromKey("name");
				if (nameD.type() != DL::Data::T_String)
					continue;

				File file;
				file.Name = nameD.getString();

				for (size_t i = 0; i < entry.anonymousCount(); ++i) {
					DL::Data channelD = entry.at(i);

					if (channelD.type() == DL::Data::T_Group
						&& channelD.getGroup().id() == "channel") {
						DL::DataGroup channel = channelD.getGroup();
						DL::Data typeD		  = channel.getFromKey("type");
						DL::Data colorD		  = channel.getFromKey("color");
						DL::Data gammaD		  = channel.getFromKey("gamma");
						DL::Data mapperD	  = channel.getFromKey("mapper");

						if (typeD.type() != DL::Data::T_String)
							continue;

						std::string type = typeD.getString();
						std::transform(type.begin(), type.end(), type.begin(), ::tolower);

						ToneColorMode tcm = TCM_SRGB;
						if (colorD.type() == DL::Data::T_String) {
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
						if (gammaD.type() == DL::Data::T_String) {
							std::string gamma = gammaD.getString();
							std::transform(gamma.begin(), gamma.end(), gamma.begin(), ::tolower);
							if (gamma == "none")
								tgm = TGM_None;
						}

						ToneMapperMode tmm = TMM_None;
						if (mapperD.type() == DL::Data::T_String) {
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

						if (type == "rgb" || type == "color") {
							IM_ChannelSettingSpec spec;
							spec.Elements = 0; //TODO
							spec.TCM	  = tcm;
							spec.TGM	  = tgm;
							spec.TMM	  = tmm;
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

								switch (var3D) {
								default:
								case OutputBuffer::V_Position:
									spec.Name[0] = "p.x";
									spec.Name[1] = "p.y";
									spec.Name[2] = "p.z";
									break;
								case OutputBuffer::V_Normal:
									spec.Name[0] = "n.x";
									spec.Name[1] = "n.y";
									spec.Name[2] = "n.z";
									break;
								case OutputBuffer::V_NormalG:
									spec.Name[0] = "ng.x";
									spec.Name[1] = "ng.y";
									spec.Name[2] = "ng.z";
									break;
								case OutputBuffer::V_Tangent:
									spec.Name[0] = "nx.x";
									spec.Name[1] = "nx.y";
									spec.Name[2] = "nx.z";
									break;
								case OutputBuffer::V_Bitangent:
									spec.Name[0] = "ny.x";
									spec.Name[1] = "ny.y";
									spec.Name[2] = "ny.z";
									break;
								case OutputBuffer::V_View:
									spec.Name[0] = "v.x";
									spec.Name[1] = "v.y";
									spec.Name[2] = "v.z";
									break;
								case OutputBuffer::V_UVW:
									spec.Name[0] = "uv.u";
									spec.Name[1] = "uv.v";
									spec.Name[2] = "uv.w";
									break;
								case OutputBuffer::V_DPDT:
									spec.Name[0] = "dpdt.x";
									spec.Name[1] = "dpdt.y";
									spec.Name[2] = "dpdt.z";
									break;
								}

								file.Settings3D.push_back(spec);
							} else if (var1D != OutputBuffer::V_1D_COUNT) {
								IM_ChannelSetting1D spec;
								spec.TMM	  = tmm;
								spec.Variable = var1D;

								switch (var1D) {
								default:
								case OutputBuffer::V_Depth:
									spec.Name = "depth";
									break;
								case OutputBuffer::V_Time:
									spec.Name = "time";
									break;
								case OutputBuffer::V_Material:
									spec.Name = "mat";
									break;
								}
								file.Settings1D.push_back(spec);
							} else if (varCounter != OutputBuffer::V_COUNTER_COUNT) {
								IM_ChannelSettingCounter spec;
								spec.TMM	  = tmm;
								spec.Variable = varCounter;

								switch (var1D) {
								default:
								case OutputBuffer::V_ID:
									spec.Name = "id";
									break;
								case OutputBuffer::V_Samples:
									spec.Name = "samples";
									break;
								}
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
				if (nameD.type() == DL::Data::T_String) {
					FileSpectral spec;
					spec.Name	 = nameD.getString();
					spec.Compress = compressD.type() == DL::Data::T_Bool ? compressD.getBool() : true;
					mSpectralFiles.push_back(spec);
				}
			}
		}
	}
}

void OutputSpecification::save(const std::shared_ptr<RenderContext>& renderer,
							   ToneMapper& toneMapper, bool force) const
{
	if (!force && !boost::filesystem::create_directory(mWorkingDir + LOCK_IMG_FILE_NAME))
		return;

	std::string resultDir = "/results";
	if (renderer->index() > 0) {
		std::stringstream stream;
		stream << resultDir << "_" << renderer->index();
		resultDir = stream.str();
	}

	const std::string dir = mWorkingDir + resultDir;
	boost::filesystem::create_directory(dir); // Doesn't matter if it works or not

	for (const File& f : mFiles) {
		const std::string filename = dir + "/" + f.Name + ".exr";
		if (!mImageWriter.save(toneMapper, filename,
							   f.SettingsSpectral, f.Settings1D, f.SettingsCounter, f.Settings3D))
			PR_LOG(L_ERROR) << "Couldn't save image file " << filename << std::endl;

		if (force)
			PR_LOG(L_INFO) << "Saved file " << filename << std::endl;
	}

	for (const FileSpectral& f : mSpectralFiles) {
		const std::string filename = dir + "/" + f.Name + ".spec";
		if (!mImageWriter.save_spectral(filename, renderer->output()->getSpectralChannel(), f.Compress))
			PR_LOG(L_ERROR) << "Couldn't save spectral file " << filename << std::endl;

		if (force)
			PR_LOG(L_INFO) << "Saved file " << filename << std::endl;
	}

	if (!force && !boost::filesystem::remove(mWorkingDir + LOCK_IMG_FILE_NAME)) // Remove it now
		PR_LOG(L_ERROR) << "Couldn't delete lock directory " << mWorkingDir << LOCK_IMG_FILE_NAME << "!" << std::endl;
}
} // namespace PR
