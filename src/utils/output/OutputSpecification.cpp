#include "OutputSpecification.h"
#include "renderer/Renderer.h"
#include "Logger.h"

#include <boost/filesystem.hpp>

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "DataArray.h"
#include "Data.h"
#include "SourceLogger.h"

namespace PRU
{
	using namespace PR;

	OutputSpecification::OutputSpecification() :
		mRenderer(nullptr)
	{
	}

	OutputSpecification::~OutputSpecification()
	{
		deinit();
	}

	void OutputSpecification::init(Renderer* renderer)
	{
		PR_ASSERT(renderer);
		mRenderer = renderer;
		mImageWriter.init(renderer);
		
		if(mRenderer)
		{
			// Setup lock directory
			if(!boost::filesystem::create_directory(mRenderer->workingDir() + "/.lock"))
				PR_LOGGER.logf(L_Warning, M_System,
					"Couldn't create lock directory '%s/.lock'. Maybe already running?", mRenderer->workingDir().c_str());

			if(!boost::filesystem::remove(mRenderer->workingDir() + "/.img_lock"))// Remove it now
				PR_LOGGER.logf(L_Error, M_System,
					"Couldn't delete lock directory '%s/.img_lock'!", mRenderer->workingDir().c_str());
		}
	}

	void OutputSpecification::deinit()
	{
		mImageWriter.deinit();

		for(const File& file : mFiles)
		{
			if(file.SettingsSpectral)
				delete file.SettingsSpectral;
		}
		mFiles.clear();
		mSpectralFiles.clear();

		if(!mRenderer)
			return;

		std::string f = mRenderer->workingDir() + "/.lock";
		if(!boost::filesystem::remove(f))
			PR_LOGGER.logf(L_Error, M_System,
				"Couldn't delete lock directory '%s'!", f.c_str());
				
		mRenderer = nullptr;
	}

	void OutputSpecification::setup()
	{
		OutputMap* output = mRenderer->output();

		for(File& file : mFiles)
		{
			if(file.SettingsSpectral)
				file.SettingsSpectral->Channel = output->getSpectralChannel();
			
			for(IM_ChannelSetting3D& cs3d : file.Settings3D)
			{
				Output3D* channel = output->getChannel(cs3d.Variable);
				if(!channel)
				{
					channel = new Output3D(mRenderer);
					output->registerChannel(cs3d.Variable, channel);
				}
				
				cs3d.Channel = channel;
			}
			
			for(IM_ChannelSetting1D& cs1d : file.Settings1D)
			{
				Output1D* channel = output->getChannel(cs1d.Variable);
				if(!channel)
				{
					channel = new Output1D(mRenderer);
					output->registerChannel(cs1d.Variable, channel);
				}
				
				cs1d.Channel = channel;
			}
		}

		for(FileSpectral& file : mSpectralFiles)
			file.Spectral = output->getSpectralChannel();
	}

	OutputMap::Variable1D typeToVariable1D(const std::string& str)
	{
		if(str == "depth" || str == "d")
			return OutputMap::V_Depth;
		else if(str == "time" || str == "t")
			return OutputMap::V_Time;
		else if(str == "quality" || str == "qual" || str == "q")
			return OutputMap::V_Quality;
		else if(str == "material" || str == "mat" || str == "m")
			return OutputMap::V_Material;
		else if(str == "id")
			return OutputMap::V_ID;
		else if(str == "samples" || str == "s")
			return OutputMap::V_Samples;
		else
			return OutputMap::V_1D_COUNT;// AS UNKNOWN
	}

	OutputMap::Variable3D typeToVariable3D(const std::string& str)
	{
		if(str == "position" || str == "pos" || str == "p")
			return OutputMap::V_Position;
		else if(str == "normal" || str == "norm" || str == "n")
			return OutputMap::V_Normal;
		else if(str == "geometric_normal" || str == "ng")
			return OutputMap::V_NormalG;
		else if(str == "tangent" || str == "tan" || str == "nx")
			return OutputMap::V_Tangent;
		else if(str == "bitangent" || str == "binormal" || str == "bi" || str == "ny")
			return OutputMap::V_Bitangent;
		else if(str == "view" || str == "v")
			return OutputMap::V_View;
		else if(str == "uv" || str == "texture" || str == "uvw" || str == "tex")
			return OutputMap::V_UVW;
		else if(str == "dpdu")
			return OutputMap::V_DPDU;
		else if(str == "dpdv")
			return OutputMap::V_DPDV;
		else if(str == "dpdw")
			return OutputMap::V_DPDW;
		else if(str == "dpdx")
			return OutputMap::V_DPDX;
		else if(str == "dpdy")
			return OutputMap::V_DPDY;
		else if(str == "dpdz")
			return OutputMap::V_DPDZ;
		else if(str == "velocity" || str == "movement" || str == "dpdt")
			return OutputMap::V_DPDT;
		else
			return OutputMap::V_3D_COUNT;// AS UNKNOWN
	}

	void OutputSpecification::parse(SceneLoader* loader, Environment* env, DL::DataGroup* group)
	{
		for (size_t i = 0; i < group->unnamedCount(); ++i)
		{
			DL::Data* dataD = group->at(i);

			if (dataD && dataD->isType() == DL::Data::T_Group)
			{
				DL::DataGroup* entry = dataD->getGroup();

				if (entry->id() == "output")
				{
					DL::Data* nameD = entry->getFromKey("name");
					if(!nameD || nameD->isType() != DL::Data::T_String)
						continue;
					
					File file;
					file.SettingsSpectral = nullptr;
					file.Name = nameD->getString();
					
					for (size_t i = 0; i < entry->unnamedCount(); ++i)
					{
						DL::Data* channelD = entry->at(i);

						if (channelD && channelD->isType() == DL::Data::T_Group 
							&& channelD->getGroup()->id() == "channel")
						{
							DL::DataGroup* channel = channelD->getGroup();
							DL::Data* typeD = channel->getFromKey("type");
							DL::Data* colorD = channel->getFromKey("color");
							DL::Data* gammaD = channel->getFromKey("gamma");
							DL::Data* mapperD = channel->getFromKey("mapper");

							if(!typeD || typeD->isType() != DL::Data::T_String)
								continue;
							
							std::string type = typeD->getString();
							std::transform(type.begin(), type.end(), type.begin(), ::tolower);

							ToneColorMode tcm = TCM_SRGB;
							if(colorD && colorD->isType() == DL::Data::T_String)
							{
								std::string color = colorD->getString();
								std::transform(color.begin(), color.end(), color.begin(), ::tolower);
								if(color == "xyz")
									tcm = TCM_XYZ;
								else if(color == "norm_xyz")
									tcm = TCM_XYZ_NORM;
								else if(color == "lum" || color == "luminance" || color == "gray")
									tcm = TCM_LUMINANCE;
							}

							ToneGammaMode tgm = TGM_SRGB;
							if(gammaD && gammaD->isType() == DL::Data::T_String)
							{
								std::string gamma = gammaD->getString();
								std::transform(gamma.begin(), gamma.end(), gamma.begin(), ::tolower);
								if(gamma == "none")
									tgm = TGM_None;
							}

							ToneMapperMode tmm = TMM_None;
							if(mapperD && mapperD->isType() == DL::Data::T_String)
							{
								std::string mapper = mapperD->getString();
								std::transform(mapper.begin(), mapper.end(), mapper.begin(), ::tolower);
								if(mapper == "reinhard")
									tmm = TMM_Simple_Reinhard;
								else if(mapper == "clamp")
									tmm = TMM_Clamp;
								else if(mapper == "absolute" || mapper == "abs")
									tmm = TMM_Abs;
								else if(mapper == "positive" || mapper == "pos")
									tmm = TMM_Positive;
								else if(mapper == "negative" || mapper == "neg")
									tmm = TMM_Negative;
								else if(mapper == "spherical")
									tmm = TMM_Spherical;
								else if(mapper == "normalize" || mapper == "normalized" || mapper == "norm")
									tmm = TMM_Normalized;
							}

							if(type == "rgb" || type == "color")
							{
								if(file.SettingsSpectral)
									delete file.SettingsSpectral;
								
								file.SettingsSpectral = new IM_ChannelSettingSpec;
								file.SettingsSpectral->Channel = nullptr;
								file.SettingsSpectral->Elements = 0;//TODO
								file.SettingsSpectral->TCM = tcm;
								file.SettingsSpectral->TGM = tgm;
								file.SettingsSpectral->TMM = tmm;
							}
							else
							{
								OutputMap::Variable3D var3D = typeToVariable3D(type);
								if(var3D != OutputMap::V_3D_COUNT)
								{
									IM_ChannelSetting3D spec;
									spec.Channel = nullptr;
									spec.Elements = 0;//TODO
									spec.TMM = tmm;
									spec.Variable = var3D;

									switch(var3D)
									{
										default:
										case OutputMap::V_Position:
											spec.Name[0] = "p_x";
											spec.Name[1] = "p_y";
											spec.Name[2] = "p_z";
											break;
										case OutputMap::V_Normal:
											spec.Name[0] = "n_x";
											spec.Name[1] = "n_y";
											spec.Name[2] = "n_z";
											break;
										case OutputMap::V_NormalG:
											spec.Name[0] = "ng_x";
											spec.Name[1] = "ng_y";
											spec.Name[2] = "ng_z";
											break;
										case OutputMap::V_Tangent:
											spec.Name[0] = "nx_x";
											spec.Name[1] = "nx_y";
											spec.Name[2] = "nx_z";
											break;
										case OutputMap::V_Bitangent:
											spec.Name[0] = "ny_x";
											spec.Name[1] = "ny_y";
											spec.Name[2] = "ny_z";
											break;
										case OutputMap::V_View:
											spec.Name[0] = "v_x";
											spec.Name[1] = "v_y";
											spec.Name[2] = "v_z";
											break;
										case OutputMap::V_UVW:
											spec.Name[0] = "uv_u";
											spec.Name[1] = "uv_v";
											spec.Name[2] = "uv_w";
											break;
										case OutputMap::V_DPDU:
											spec.Name[0] = "dpdu_x";
											spec.Name[1] = "dpdu_y";
											spec.Name[2] = "dpdu_z";
											break;
										case OutputMap::V_DPDV:
											spec.Name[0] = "dpdv_x";
											spec.Name[1] = "dpdv_y";
											spec.Name[2] = "dpdv_z";
											break;
										case OutputMap::V_DPDW:
											spec.Name[0] = "dpdw_x";
											spec.Name[1] = "dpdw_y";
											spec.Name[2] = "dpdw_z";
											break;
										case OutputMap::V_DPDX:
											spec.Name[0] = "dpdx_x";
											spec.Name[1] = "dpdx_y";
											spec.Name[2] = "dpdx_z";
											break;
										case OutputMap::V_DPDY:
											spec.Name[0] = "dpdy_x";
											spec.Name[1] = "dpdy_y";
											spec.Name[2] = "dpdy_z";
											break;
										case OutputMap::V_DPDZ:
											spec.Name[0] = "dpdz_x";
											spec.Name[1] = "dpdz_y";
											spec.Name[2] = "dpdz_z";
											break;
										case OutputMap::V_DPDT:
											spec.Name[0] = "dpdt_x";
											spec.Name[1] = "dpdt_y";
											spec.Name[2] = "dpdt_z";
											break;
									}

									file.Settings3D.push_back(spec);
								}
								else
								{
									OutputMap::Variable1D var1D = typeToVariable1D(type);
									if(var1D != OutputMap::V_1D_COUNT)
									{
										IM_ChannelSetting1D spec;
										spec.Channel = nullptr;
										spec.TMM = tmm;
										spec.Variable = var1D;

										switch(var1D)
										{
											default:
											case OutputMap::V_Depth:
												spec.Name = "A";
												break;
											case OutputMap::V_Time:
												spec.Name = "time";
												break;
											case OutputMap::V_Quality:
												spec.Name = "quality";
												break;
											case OutputMap::V_Material:
												spec.Name = "mat";
												break;
											case OutputMap::V_ID:
												spec.Name = "id";
												break;
											case OutputMap::V_Samples:
												spec.Name = "samples";
												break;
										}
										file.Settings1D.push_back(spec);
									}
									else
									{
										PR_LOGGER.logf(L_Error, M_Scene,
											"Unknown channel type %s!", type.c_str());
									}
								}
							}
						}
					}
					
					mFiles.push_back(file);
				}
				else if (entry->id() == "output_spectral")
				{
					DL::Data* nameD = entry->getFromKey("name");
					if(nameD && nameD->isType() == DL::Data::T_String)
					{
						FileSpectral spec;
						spec.Name = nameD->getString();
						mSpectralFiles.push_back(spec);
					}
				}
			}
		}
	}

	void OutputSpecification::save(ToneMapper& toneMapper, bool force) const
	{
		if(!force && !boost::filesystem::create_directory(mRenderer->workingDir() + "/.img_lock"))
			return;

		for(const File& f: mFiles)
		{
			const std::string filename = mRenderer->workingDir() + "/" + f.Name + ".exr";
			if(!mImageWriter.save(toneMapper, filename,
				f.SettingsSpectral, f.Settings1D, f.Settings3D))
				PR_LOGGER.logf(L_Error, M_System,
					"Couldn't save image file '%s'!", filename.c_str());
		}

		for(const FileSpectral& f: mSpectralFiles)
		{
			const std::string filename = mRenderer->workingDir() + "/" + f.Name + ".spec";
			if(!mImageWriter.save_spectral(filename, f.Spectral))
				PR_LOGGER.logf(L_Error, M_System,
					"Couldn't save spectral file '%s'!", filename.c_str());
		}

		if(!force && !boost::filesystem::remove(mRenderer->workingDir() + "/.img_lock"))// Remove it now
			PR_LOGGER.logf(L_Error, M_System,
				"Couldn't delete lock directory '%s/.img_lock'!", mRenderer->workingDir().c_str());
	}
}